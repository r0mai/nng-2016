#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <chrono>

// Node

struct Node {
	Node() {
		next = prev = this;
	}

	Node* next = nullptr;
	Node* prev = nullptr;
};

void Unlink(Node* node) {
	node->prev->next = node->next;
	node->next->prev = node->prev;
	node->next = node;
	node->prev = node;
}

void Insert(Node* p, Node* q) {
	Unlink(q);
	p->prev->next = q;
	q->prev = p->prev;
	q->next = p;
	p->prev = q;
}


// Block

enum Dir {
	kLeft,
	kRight,
	kTop,
	kBottom
};

struct Block {
	Block* buddy[4] = {};
	Node older;
	Node newer;

	int neighbors = 0;
	int height = 0;
	int row = -1;
	int col = -1;
};


using NodePtr = Node Block::*;


Block* GetOlderBlock(Node* node) {
	NodePtr ptr = &Block::older;
	Block* obj = nullptr;
	char* delta = reinterpret_cast<char*>(&(obj->*ptr));
	return reinterpret_cast<Block*>(
		reinterpret_cast<const char*>(node) - delta);
}

Block* GetNewerBlock(Node* node) {
	NodePtr ptr = &Block::newer;
	Block* obj = nullptr;
	char* delta = reinterpret_cast<char*>(&(obj->*ptr));
	return reinterpret_cast<Block*>(
		reinterpret_cast<const char*>(node) - delta);
}


bool IsOlder(Block& block) {
	return (block.height > 0 && block.height < 5 &&
		block.height == block.neighbors + 1);
}


bool IsNewer(Block& block) {
	return (block.height == 1 || (block.height == 5 && block.neighbors < 4));
}


template<typename Function>
void ForEachNeighbor(Block& block, Function fn) {
	if (block.buddy[kLeft])		{ fn(*block.buddy[kLeft], kRight); }
	if (block.buddy[kRight])	{ fn(*block.buddy[kRight], kLeft); }
	if (block.buddy[kTop])		{ fn(*block.buddy[kTop], kBottom); }
	if (block.buddy[kBottom])	{ fn(*block.buddy[kBottom], kTop); }
}


// Parcel

using BlockVec = std::vector<Block>;
using Layout = std::vector<BlockVec>;

class Parcel {
public:
	void Load(const std::string& fname) {
		std::ifstream f(fname);
		f >> rows_ >> cols_;

		layout_.resize(rows_);
		for (int row = 0; row < rows_; ++row) {
			layout_[row].resize(cols_);
			for (int col = 0; col < cols_; ++col) {
				int h;
				f >> h;
				InitBlock(row, col, h);
			}
		}
		InitEdges();
	}

	void InitEdges() {
		int count = 0;
		for (auto& vec : layout_) {
			for (auto& block : vec) {
				if (IsOlder(block)) {
					Insert(&older_, &block.older);
					++count;
				} else if (IsNewer(block)) {
					Insert(&newer_, &block.newer);
					++count;
				}
			}
		}
		std::cout << "Edges: " << count << std::endl;
	}

	void InitBlock(int row, int col, int h) {
		// assume left and top are already initialized
		auto& block = layout_[row][col];
		block.row = row;
		block.col = col;

		if (h == 0) {
			return;
		}
		block.height = (h > 1 ? h : 5);	// rewrite rule

		if (col > 0) {
			auto& left = layout_[row][col - 1];
			if (left.height) {
				left.buddy[kRight] = &block;
				block.buddy[kLeft] = &left;
				++left.neighbors;
				++block.neighbors;
			}
		}

		if (row > 0) {
			auto& top = layout_[row - 1][col];
			if (top.height) {
				top.buddy[kBottom] = &block;
				block.buddy[kTop] = &top;
				++top.neighbors;
				++block.neighbors;
			}
		}
	}

	void ResetBlock(Block& block) {
		block.height = 0;
		block.neighbors = 0;
		for (int i = 0; i < 4; ++i) {
			block.buddy[i] = nullptr;
		}

		Unlink(&block.older);
		Unlink(&block.newer);
	}

	void ErodeBlock(Block& block) {
		ForEachNeighbor(block, [&](Block& nb, Dir dir) {
			--nb.neighbors;
			nb.buddy[dir] = nullptr;
			if (IsOlder(nb)) {
				Insert(&older_, &nb.older);
			} else if (IsNewer(nb)) {
				Insert(&newer_, &nb.newer);
			}
		});

		ResetBlock(block);
	}

	void Erode() {
		int count = 0;
		while (older_.next != &older_) {
			Block& block = *GetOlderBlock(older_.next);
			ErodeBlock(block);
			++count;
		}
		steps_ += count;
		// std::cout << count << std::endl;
	}

	void UnbuildBlock(Block& block) {
		ForEachNeighbor(block, [&](Block& nb, Dir dir) {
			--nb.neighbors;
			--nb.height;
			nb.buddy[dir] = nullptr;
			if (IsOlder(nb)) {
				Insert(&older_, &nb.older);
			} else if (IsNewer(nb)) {
				Insert(&newer_, &nb.newer);
			}
		});

		ResetBlock(block);
	}

	void Unbuild() {
		int count = 0;
		while (newer_.next != &newer_) {
			Block& block = *GetNewerBlock(newer_.next);
			++count;
			UnbuildBlock(block);
		}
		steps_ += count;
		// std::cout << count << std::endl;
	}


	void Visual() const {
		bool show_height = false;
		bool show_neighbors = false;
		bool show_steps = false;;
		bool show_buildings = false;

		// show_height = true;
		if (show_height) {
			std::cout << "Height:" << std::endl;

			for (const auto& vec : layout_) {
				bool first = true;
				for (const auto& block : vec) {
					if (first) {
						first = false;
					} else {
						std::cout << " ";
					}
					std::cout << block.height;
				}
				std::cout << std::endl;
			}
		}

		// show_neighbors = true;
		if (show_neighbors) {
			std::cout << "Neighbors:" << std::endl;

			for (const auto& vec : layout_) {
				bool first = true;
				for (const auto& block : vec) {
					if (first) {
						first = false;
					} else {
						std::cout << " ";
					}
					std::cout << block.neighbors;
				}
				std::cout << std::endl;
			}
		}

		show_steps = true;
		if (show_steps) {
			std::cout << "Steps: " << steps_ << std::endl;
		}

		show_buildings = true;
		if (show_buildings) {
			int bs = 0;
			for (const auto& vec : layout_) {
				for (const auto& block : vec) {
					if (block.height > 0) { ++bs; }
				}
			}
			std::cout << "Buildings: " << bs << std::endl;
		}
	}

	bool HasOlder() const {
		return older_.next != &older_;
	}

	bool HasNewer() const {
		return newer_.next != &newer_;
	}


private:
	int rows_ = 0;
	int cols_ = 0;
	Node older_;
	Node newer_;
	Layout layout_;
	int steps_ = 0;
};


using Clock = std::chrono::high_resolution_clock;

double DeltaT(Clock::time_point t1, Clock::time_point t2) {
	using Duration = std::chrono::duration<double>;
	return std::chrono::duration_cast<Duration>(t2 - t1).count();
}


int main() {
	Parcel p;
	p.Load("test100.map");

	auto start_t = Clock::now();
	bool loop = true;
	while (loop) {
		loop = false;
		if (p.HasNewer()) {
			p.Unbuild();
			loop = true;
		}
		if (p.HasOlder()) {
			p.Erode();
			loop = true;
		}
	}
	auto end_t = Clock::now();
	p.Visual();

	std::cout << "Elapsed: " << DeltaT(start_t, end_t) << "s" << std::endl;

	return 0;
}
