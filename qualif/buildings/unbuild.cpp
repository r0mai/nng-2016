#include <iostream>
#include <fstream>
#include <algorithm>
#include <vector>


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
	Node edge;

	int neighbors = 0;
	int height = 0;
	int row = -1;
	int col = -1;
};


using NodePtr = Node Block::*;


Block* FromEdgeNode(Node* node) {
	NodePtr edge = &Block::edge;
	Block* obj = nullptr;
	char* delta = reinterpret_cast<char*>(&(obj->*edge));
	return reinterpret_cast<Block*>(
		reinterpret_cast<const char*>(node) - delta);
}


template<typename Function>
void ForEachNeighbor(Block& block, Function fn) {
	if (block.buddy[kLeft]) 	{ fn(*block.buddy[kLeft], kRight); }
	if (block.buddy[kRight]) 	{ fn(*block.buddy[kRight], kLeft); }
	if (block.buddy[kTop]) 		{ fn(*block.buddy[kTop], kBottom); }
	if (block.buddy[kBottom]) 	{ fn(*block.buddy[kBottom], kTop); }
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
				if (block.neighbors > 0 && block.neighbors < 4) {
					Insert(&edge_, &block.edge);
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

	Node* ResetBlock(Block& block) {
		block.height = 0;
		block.neighbors = 0;
		for (int i = 0; i < 4; ++i) {
			block.buddy[i] = nullptr;
		}

		auto p = block.edge.prev;
		Unlink(&block.edge);
		return p;
	}

	Node* ErodeBlock(Block& block) {
		ForEachNeighbor(block, [&](Block& nb, Dir dir) {
			--nb.neighbors;
			nb.buddy[dir] = nullptr;
			if (nb.height == nb.neighbors + 1 || nb.height == 5) {
				Insert(&edge_, &nb.edge);
			}
		});

		return ResetBlock(block);
	}

	int Erode() {
		int count = 0;
		int result = 0;
		for (Node* p = edge_.next; p != &edge_; p = p->next) {
			Block& block = *FromEdgeNode(p);
			++count;

			if (block.height == 5) {
				block.height = 1;
				++result;
			} else if (block.height == block.neighbors + 1) {
				p = ErodeBlock(block);
			}
		}
		steps_ += count;
		return result;
	}

	Node* UnbuildBlock(Block& block) {
		ForEachNeighbor(block, [&](Block& nb, Dir dir) {
			Insert(&edge_, &nb.edge);
			--nb.neighbors;
			--nb.height;
			nb.buddy[dir] = nullptr;
		});

		return ResetBlock(block);
	}

	void Unbuild() {
		int count = 0;
		for (Node* p = edge_.next; p != &edge_; p = p->next) {
			Block& block = *FromEdgeNode(p);
			++count;

			if (block.height == 1) {
				p = UnbuildBlock(block);
			}
		}
		steps_ += count;
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

		show_buildings = false;
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

private:
	int rows_ = 0;
	int cols_ = 0;
	Node edge_;
	Layout layout_;
	int steps_ = 0;
};


int main() {
	Parcel p;
	p.Load("test1000.map");

	while (p.Erode()) {
		p.Unbuild();
	}
	p.Visual();
	return 0;
}
