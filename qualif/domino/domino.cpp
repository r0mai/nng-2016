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
using IntMap = std::unordered_map<int, int>;

struct Entry {
	IntSet prefix_of;
	IntSet suffix_of;
};

struct Word {
	int index = -1;
	int color = -1;
	int prev = -1;
	int next = -1;
	int overlap = 0;
};

using PrefixMap = std::unordered_map<std::string, IntSet>;
using EntryMap = std::unordered_map<std::string, Entry>;
using WordVec = std::vector<Word>;

struct MergeState {
	std::string suffix;
	size_t total_len = 0;
	size_t merged_len = 0;
	std::ostream* output = nullptr;

	IntSet unused;
	StringVec vec;
	EntryMap emap;
	WordVec words;
	IntMap cmap;

	int colors = 0;
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

EntryMap CreateEntries(const StringVec& vec) {
	EntryMap emap;

	for (size_t i = 0, ie = vec.size(); i != ie; ++i) {
		ForEachPrefix(vec[i], [&](const std::string& s) {
			emap[s].prefix_of.insert(i);
		});

		ForEachSuffix(vec[i], [&](const std::string& s) {
			emap[s].suffix_of.insert(i);
		});
	}

	return emap;
}

IntSet FindPrefixWords(const StringVec& vec, const EntryMap& emap) {
	IntSet pwords;
	size_t index = -1;
	for (const auto& s : vec) {
		++index;
		auto it = emap.find(s);
		if (it == end(emap)) { continue; }
		if (it->second.prefix_of.size() > 1) {
			pwords.insert(index);
		}
	}
	return pwords;
}

void RemoveRefs(const StringVec& vec, int p_index, int s_index, EntryMap& emap)
{
	ForEachPrefix(vec[p_index], [&](const std::string& s) {
		auto it = emap.find(s);
		if (it != end(emap)) { it->second.prefix_of.erase(p_index); }
	});

	ForEachSuffix(vec[s_index], [&](const std::string& s) {
		auto it = emap.find(s);
		if (it != end(emap)) { it->second.suffix_of.erase(s_index); }
	});
}

void Init(MergeState& mm, const std::string& fname) {
	mm.vec = ReadLines(fname);
	mm.emap = CreateEntries(mm.vec);
	mm.words.resize(mm.vec.size());

	for (size_t i = 0, ie = mm.vec.size(); i != ie; ++i) {
		mm.unused.insert(i);
		mm.words[i].index = i;
	}
}

bool CanMerge(const MergeState& mm, int p, int q) {
	// self-merge
	if (p == q) {
		return false;
	}

	auto p_color = mm.words[p].color;
	auto q_color = mm.words[q].color;

	// one is uncolored
	if (p_color == -1 || q_color == -1) {
		return true;
	}

	// cannot merge with same color
	return mm.cmap.find(p_color) != mm.cmap.find(q_color);
}

void Merge(MergeState& mm, int p, int q, int overlap) {
	auto& p_color = mm.words[p].color;
	auto& q_color = mm.words[q].color;

	if (p_color == -1 && q_color == -1) {
		p_color = q_color = mm.colors++;
		mm.cmap[p_color] = p_color;
	} else if (p_color == -1 || q_color == -1) {
		p_color = q_color = std::max(p_color, q_color);
	} else {
		assert(p_color != q_color);
		// only recolorize in map
		for (auto& cc : mm.cmap) {
			if (cc.second == p_color) {
				cc.second = q_color;
			}
		}
	}

	mm.words[p].next = q;
	mm.words[q].prev = p;
	mm.words[q].overlap = overlap;
}

void Solve(MergeState& mm, const std::string& fname) {
	StringVec kvec;
	std::ofstream f(fname);
	mm.output = &f;

	kvec.reserve(mm.emap.size());
	for (const auto& e : mm.emap) { kvec.push_back(e.first); }
	std::sort(kvec.begin(), kvec.end(), CompareBySize);
	std::reverse(kvec.begin(), kvec.end());

	int count = 0;
	for (const auto& k : kvec) {
		auto& entry = mm.emap[k];
		while (!entry.suffix_of.empty() && !entry.prefix_of.empty()) {
			int first = -1;
			int second = -1;
			bool found = false;

			for (auto p : entry.suffix_of) {
				for (auto q : entry.prefix_of) {
					if (CanMerge(mm, p, q)) {
						first = p;
						second = q;
						found = true;
						break;
					}
				}
				if (found) {
					break;
				}
			}

			if (found) {
				Merge(mm, first, second, k.size());
				RemoveRefs(mm.vec, second, first, mm.emap);
			} else {
				break;
			}
		}
		++count;
		std::cout << count << std::endl;
	}

	while (!mm.unused.empty()) {
		auto p = *begin(mm.unused);
		int pmin = p;
		while (mm.words[p].prev != -1) {
			auto q = p;
			p = mm.words[p].prev;
			if (p == pmin) {
				std::cout << "breaking circle" << std::endl;
				mm.words[p].next = -1;
				mm.words[q].prev = -1;
				mm.words[q].overlap = 0;
				p = q;
			}
		}

		for (; p != -1; p = mm.words[p].next) {
			mm.unused.erase(p);
			mm.total_len += mm.vec[p].size();
			mm.merged_len += mm.vec[p].size() - mm.words[p].overlap;
			f << mm.vec[p] << std::endl;
		}
	}

	std::cout << mm.merged_len << " (approx) / " << mm.total_len << std::endl;
	f.close();
}

int Overlap(const std::string& u, const std::string& v) {
	for (int i = std::min(u.size(), v.size()); i > 0; --i) {
		auto su = u.substr(u.size() - i);
		auto sv = v.substr(0, i);
		assert(su.size() == sv.size());
		if (su == sv) {
			return i;
		}
	}
	return 0;
}

void Validate(const std::string& fname) {
	std::ifstream f(fname);
	std::string line;
	StringVec vec;

	while (getline(f, line)) {
		Strip(line);
		vec.push_back(line);
	}

	std::string last_word;
	int total_len = 0;
	int merged_len = 0;
	for (const auto& word : vec) {
		int overlap = Overlap(last_word, word);
		// std::cout << word << " " << overlap << std::endl;

		total_len += word.size();
		merged_len += word.size() - overlap;
		last_word = word;
	}
	std::cout << merged_len << " / " << total_len << std::endl;
}


// Main

int main() {
	MergeState mm;
	// Init(mm, "words_final.txt");
	// Solve(mm, "solution.txt");

	Validate("solution.txt");
	return 0;
}
