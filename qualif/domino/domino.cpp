#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <unordered_map>
#include <algorithm>
#include <climits>

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

	IntSet pwords;
	IntSet unused;
	StringVec vec;
	PrefixMap pmap;
};


struct Mergeable {
	std::string str;
	size_t overlap = 0;
	int index = -1;
};


// Algo

void Strip(std::string& str) {
	while (!str.empty() && str.back() == '\r') {
		str.pop_back();
	}
}

bool CompareBySize(const std::string& s1, const std::string& s2) {
	return (s1.size() < s2.size() ||
		(s1.size() == s2.size() && s1 < s2));
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

IntSet FindPrefixWords(const StringVec& vec, const PrefixMap& pmap) {
	IntSet pwords;
	size_t index = -1;
	for (const auto& s : vec) {
		++index;
		auto it = pmap.find(s);
		if (it == end(pmap)) { continue; }
		if (it->second.size() > 1) {
			pwords.insert(index);
		}
	}
	return pwords;
}

void RemoveRefs(const StringVec& vec, int index, PrefixMap& pmap)
{
	ForEachPrefix(vec[index], [&](const std::string& s) {
		auto it = pmap.find(s);
		if (it != end(pmap)) { it->second.erase(index); }
	});
}

void Merge(size_t index, size_t overlap, MergeState& mm) {
	const auto& w = mm.vec[index];
	auto cc = mm.suffix + w.substr(overlap);
	mm.suffix = cc.substr(cc.size() - std::min(mm.keep_size, cc.size()));
	mm.total_len += w.size();
	mm.merged_len += w.size() - overlap;

	if (mm.output) {
		*mm.output << w << std::endl;
	}

	mm.unused.erase(index);
	mm.pwords.erase(index);
	RemoveRefs(mm.vec, index, mm.pmap);
}

int Fitness(const Mergeable& w) {
	if (w.index < 0) {
		return INT_MIN;
	}
	return w.overlap;
}

bool IsEmpty(const MergeState& mm) {
	return mm.unused.empty();
}

int Pick(const MergeState& mm) {
	if (!mm.pwords.empty()) {
		return *mm.pwords.rbegin();
	}
	return *begin(mm.unused);
}

void Solve(MergeState& mm) {
	std::ofstream f("solution.txt");
	mm.output = &f;

	for (size_t i = 0, ie = mm.vec.size(); i != ie; ++i) {
		mm.keep_size = std::max(mm.keep_size, mm.vec[i].size());
		mm.unused.insert(i);
	}

	while (!IsEmpty(mm)) {
		int p = Pick(mm);
		Merge(p, 0, mm);

		bool can_merge = true;
		while (can_merge) {
			Mergeable best;

			can_merge = false;
			for (size_t i = 0, ie = mm.suffix.size(); i != ie; ++i) {
				auto str = mm.suffix.substr(i);
				auto it = mm.pmap.find(str);
				if (it != end(mm.pmap) && !it->second.empty()) {
					Mergeable w;
					w.index = *begin(it->second);
					w.str = mm.vec[w.index];
					w.overlap = str.size();
					if (Fitness(w) > Fitness(best)) {
						best = w;
					}
					can_merge = true;
				}
			}

			if (can_merge) {
				int q = best.index;
				Merge(q, best.overlap, mm);
			}
		}
	}

	std::cout << mm.merged_len << " (" << mm.total_len << ")" << std::endl;
	f.close();
}


// Main

int main() {
	MergeState mm;
	mm.vec = ReadLines("words_final.txt");
	mm.pmap = CreatePrefixes(mm.vec);
	mm.pwords = FindPrefixWords(mm.vec, mm.pmap);
	Solve(mm);
	return 0;
}

