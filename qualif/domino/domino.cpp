#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>


// Data

using StringVec = std::vector<std::string>;
using IntSet = std::set<int>;
using PrefixMap = std::unordered_map<std::string, IntSet>;

struct MergeState {
	std::string suffix;
	size_t keep_size = 0;
	size_t total_len = 0;
	size_t merged_len = 0;
	std::ostream* output = nullptr;
};


// Algo

void Strip(std::string& str) {
	while (!str.empty() && str.back() == '\r') {
		str.pop_back();
	}
}

bool CompareBySize(const std::string& s1, const std::string& s2) {
	return (s1.size() < s2.size() ||
		(s2.size() == s2.size() && s1 < s2));
}

template<typename Function>
void ForEachPrefix(const std::string& str, Function fn) {
	if (str.empty()) { return; }
	for (size_t i = 0, ie = str.size(); i != ie; ++i) {
		fn(str.substr(0, i + 1));
	}
}

template<typename Function>
void ForEachSuffix(const std::string& str, Function fn) {
	if (str.empty()) { return; }
	for (size_t i = 0, ie = str.size(); i != ie; ++i) {
		fn(str.substr(i));
	}
}

StringVec ReadLines(const std::string& fname) {
	std::ifstream f(fname);
	std::string line;
	StringVec vec;

	while (getline(f, line)) {
		Strip(line);
		vec.push_back(line);
	}

	std::sort(vec.begin(), vec.end(), CompareBySize);
	return vec;
}

PrefixMap CreatePrefixes(const StringVec& vec) {
	PrefixMap pmap;

	for (size_t i = 0, ie = vec.size(); i != ie; ++i) {
		const auto& str = vec[i];
		ForEachPrefix(str, [&](const std::string& s) {
			pmap[s].insert(i);
		});
	}

	return pmap;
}

void RemoveRefs(const StringVec& vec, int index, PrefixMap& pmap)
{
	ForEachPrefix(vec[index], [&](const std::string& s) {
		auto it = pmap.find(s);
		if (it != end(pmap)) { it->second.erase(index); }
	});
}

void Merge(const std::string w, size_t overlap, MergeState& mm) {
	auto cc = mm.suffix + w.substr(overlap);
	mm.suffix = cc.substr(cc.size() - std::min(mm.keep_size, cc.size()));
	mm.total_len += w.size();
	mm.merged_len += w.size() - overlap;

	if (mm.output) {
		*mm.output << w << std::endl;
	}
}

void Solve(const StringVec& vec, PrefixMap& pmap) {
	std::ofstream f("domino.out");

	IntSet unused;
	MergeState mm;
	mm.output = &f;

	for (size_t i = 0, ie = vec.size(); i != ie; ++i) {
		mm.keep_size = std::max(mm.keep_size, vec[i].size());
		unused.insert(i);
	}

	while (!unused.empty()) {
		int p = *unused.begin();
		unused.erase(p);
		RemoveRefs(vec, p, pmap);
		Merge(vec[p], 0, mm);

		bool can_merge = true;
		while (can_merge) {
			can_merge = false;
			for (size_t i = 0, ie = mm.suffix.size(); i != ie; ++i) {
				auto str = mm.suffix.substr(i);
				auto it = pmap.find(str);
				if (it != end(pmap) && !it->second.empty()) {
					int q = *begin(it->second);
					it->second.erase(q);
					unused.erase(q);
					RemoveRefs(vec, q, pmap);
					Merge(vec[q], str.size(), mm);
					can_merge = true;
					break;
				}
			}
		}
	}

	std::cout << mm.merged_len << " (" << mm.total_len << ")" << std::endl;
	f.close();
}


// Main

int main() {
	auto vec = ReadLines("words_final.txt");
	auto pmap = CreatePrefixes(vec);
	Solve(vec, pmap);
	return 0;
}

