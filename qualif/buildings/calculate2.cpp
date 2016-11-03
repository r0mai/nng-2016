#include "calculate.h"
#include <iostream>
#include <algorithm>
#include <cassert>


namespace {

enum Direction {
	kLeft = 0,
	kRight = 1,
	kTop = 2,
	kBottom = 3,
};

struct Block {
	bool neighbor[4] = {};
	int neighbor_count = 0;
	int height = 0;
	int index = -1;
	int col = -1;
	int row = -1;
};


bool IsOlder(const Block& block) {
	return (block.height > 1 && block.height < 5 &&
		block.height == block.neighbor_count + 1);
}

bool IsNewer(const Block& block) {
	return (block.height == 1 ||
		(block.height == 5 && block.neighbor_count < 4));
}


using BlockVec = std::vector<Block>;
using IntVec = std::vector<int>;
using CommandVec = std::vector<Command>;

class Parcel {
public:
	void Init(const Buildings& bs) {
		rows_ = bs.size();
		cols_ = bs.front().size();
		blocks_.resize(rows_ * cols_);
		for (int row = 0; row < rows_; ++row) {
			for (int col = 0; col < cols_; ++col) {
				InitBlock(row, col, bs[row][col]);
			}
		}
		InitStacks();
	}

	void InitBlock(int row, int col, int h) {
		// Note: assume left and top are already initialized
		auto index = BlockIndex(row, col);
		auto& block = blocks_[index];

		if (h == 0) {
			return;
		}

		block.index = index;
		block.height = (h > 1 ? h : 5);	// rewrite rule
		block.row = row;
		block.col = col;

		if (col > 0) {
			auto& left = blocks_[NeighborIndex(index, kLeft)];
			if (left.height) {
				left.neighbor[kRight] = true;
				block.neighbor[kLeft] = true;
				++left.neighbor_count;
				++block.neighbor_count;
			}
		}

		if (row > 0) {
			auto& top = blocks_[NeighborIndex(index, kTop)];
			if (top.height) {
				top.neighbor[kBottom] = true;
				block.neighbor[kTop] = true;
				++top.neighbor_count;
				++block.neighbor_count;
			}
		}
	}

	void InitStacks() {
		older_.reserve(rows_ * cols_);
		newer_.reserve(rows_ * cols_);

		for (auto& block : blocks_) {
			if (IsOlder(block)) {
				older_.push_back(block.index);
			} else if (IsNewer(block)) {
				newer_.push_back(block.index);
			}
		}
		std::cout << "Older: " << older_.size() << std::endl;
		std::cout << "Newer: " << newer_.size() << std::endl;
	}

	int BlockIndex(int row, int col) {
		return col + row * cols_;
	}

	int NeighborIndex(int index, Direction dir) const {
		auto mul = ((dir & 1) == 0 ? -1 : 1);
		auto diff = ((dir & 2) == 0 ? 1 : cols_);
		return mul * diff + index;
	}

	template<typename Function>
	void ForEachNeighbor(const Block& block, Function fn) {
		auto index = block.index;

		for (int k = 0; k < 4; ++k) {
			if (block.neighbor[k]) {
				// points to the current block
				// from the neighbor's perspective
				Direction current = Direction(k ^ 1);
				fn(blocks_[NeighborIndex(index, Direction(k))], current);
			}
		}
	}

	void EliminateBlock(Block& block, bool is_newer) {
		ForEachNeighbor(block, [&](Block& nb, Direction dir) {
			if (is_newer) {
				--nb.height;
			}
			--nb.neighbor_count;
			nb.neighbor[dir] = false;
			if (IsOlder(nb)) {
				older_.push_back(nb.index);
			} else if (IsNewer(nb)) {
				newer_.push_back(nb.index);
			}
		});

		block.height = 0;
		block.neighbor_count = 0;

		auto& target = (is_newer ? backward_ : forward_);
		target.push_back({block.row, block.col});
	}

	bool EliminateNewer() {
		while (!newer_.empty()) {
			auto index = newer_.back();
			auto& block = blocks_[index];
			newer_.pop_back();
			if (block.height > 0) {
				EliminateBlock(block, true);
				return true;
			}
		}
		return false;
	}

	bool EliminateOlder() {
		while (!older_.empty()) {
			auto index = older_.back();
			auto& block = blocks_[index];
			older_.pop_back();
			if (block.height > 0) {
				EliminateBlock(block, false);
				return true;
			}
		}
		return false;
	}

	bool EliminateNext() {
		return EliminateNewer() || EliminateOlder();
	}

	void Visual() const {
		// can has stdio?
		bool show_height = false;

		show_height = true;
		if (show_height) {
			std::cout << "Height:" << std::endl;

			for (const auto& block : blocks_) {
				if (block.col > 0) {
					std::cout << " ";
				} else if (block.row > 0) {
					std::cout << std::endl;
				}
				std::cout << block.height;
			}
			std::cout << std::endl;
		}
	}

	CommandVec GetCommands() const {
		CommandVec vec;
		vec.reserve(forward_.size() + backward_.size());
		vec = forward_;
		std::copy(backward_.rbegin(), backward_.rend(),
			std::back_inserter(vec));
		return vec;
	}

private:
	int rows_ = 0;
	int cols_ = 0;

	BlockVec blocks_;
	IntVec older_;
	IntVec newer_;

	CommandVec forward_;
	CommandVec backward_;
};

} // namespace


void CalculateBuildOrder(const Buildings& buildings,
	std::vector<Command>& solution)
{
	Parcel p;
	p.Init(buildings);
	// p.Visual();

	int count = 0;
	while (p.EliminateNext()) {
		// p.Visual();
		++count;
	}
	// p.Visual();
	std::cout << "Steps: " << count << std::endl;
	solution = p.GetCommands();
}
