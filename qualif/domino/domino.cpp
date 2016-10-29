#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
#include <set>
#include <unordered_map>


// Data

using StringVec = std::vector<std::string>;
using IntSet = std::set<int>;

struct Entry {
	IntSet prefix_of;
	IntSet suffix_of;
};

struct Domino {
	int index { -1 };
	int prev { -1 };
	int next { -1 };
	int color { 0 };
};

using EntryMap = std::unordered_map<std::string, Entry>;
using MergeMap = std::unordered_map<int, IntSet>;
using DominoVec = std::vector<Domino>;

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
	for (size_t i = 1, ie = str.size(); i != ie; ++i) {
		fn(str.substr(0, i));
	}
}


template<typename Function>
void ForEachSuffix(const std::string& str, Function fn) {
	if (str.empty()) { return; }
	for (size_t i = 1, ie = str.size(); i != ie; ++i) {
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
		const auto& str = vec[i];
		ForEachPrefix(str, [&](const std::string& s) {
			emap[s].prefix_of.insert(i);
		});

		ForEachSuffix(str, [&](const std::string& s) {
			emap[s].suffix_of.insert(i);
		});
	}
	return emap;
}


void RemoveRefs(const StringVec& vec, int p_index, int s_index,
	EntryMap& emap)
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


MergeMap MergeWords(const StringVec& vec, EntryMap& emap) {
	MergeMap suffix;

	int count = 0;
	for (size_t i = 0, ie = vec.size(); i != ie; ++i) {
		auto entry_it = emap.find(vec[i]);
		if (entry_it != end(emap)) {
			auto& entry = entry_it->second;
			if (!entry.suffix_of.empty()) {
				suffix[*begin(entry.suffix_of)].insert(i);
			} else {
				continue;
			}
			++count;
			RemoveRefs(vec, i, i, emap);
		}
	}
	std::cout << count << std::endl;
	return suffix;
}


DominoVec ConnectWords(const StringVec& vec, EntryMap& emap) {
	DominoVec dvec(vec.size());
	StringVec evec;

	for (size_t i = 0, ie = dvec.size(); i != ie; ++i) {
		dvec[i].index = i;
	}

	evec.reserve(emap.size());
	for (const auto& e : emap) { evec.push_back(e.first); }
	std::sort(evec.begin(), evec.end(), CompareBySize);

	for (auto i = evec.size(); i-- > 0; ) {
		const auto& k = evec[i];
		auto& entry = emap[k];
		while (!entry.suffix_of.empty() && !entry.prefix_of.empty()) {
			int d1 = *begin(entry.suffix_of);
			int d2 = *begin(entry.prefix_of);

			dvec[d1].next = d2;
			dvec[d2].prev = d1;
			dvec[d1].color = dvec[d2].color = 1;

			RemoveRefs(vec, d2, d1, emap);
		}
	}

	return dvec;
}

void Colorize(DominoVec& dvec) {
	IntSet indices;
	int next_color = 3;

	for (const auto& d : dvec) { indices.insert(d.index); }

	while (!indices.empty()) {
		int i = *begin(indices);
		indices.erase(i);
		std::cout<< i << std::endl;
		if (dvec[i].color == 1) {
			int first = i;
			while (dvec[first].prev != -1) {
				first = dvec[first].prev;
			}
			std::cout << "-\n";
			for (int p = first; p != -1; p = dvec[p].next) {
				dvec[p].color = next_color;
				indices.erase(p);
			}
			++next_color;
		}
	}

	std::cout << next_color << std::endl;
}



// Main

int main() {
	auto vec = ReadLines("words_final.txt");
	auto emap = CreateEntries(vec);
	auto mmap = MergeWords(vec, emap);
	auto dvec = ConnectWords(vec, emap);
	int count = 0;

	for (const auto& m : mmap) {
		for (auto i : m.second) {
			dvec[i].color = 2;
		}
	}

	std::cout << "--\n";
	// Colorize(dvec);

	// for (const auto& d : dvec) {
	// 	if (d.color) {
	// 		++count;
	// 	} else {
	// 		std::cout << vec[d.index] << std::endl;
	// 	}
	// }

	std::cout << count << std::endl;
	return 0;
}
