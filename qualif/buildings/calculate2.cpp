// do not include any local headers here
#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <cstddef>


using Command = std::pair<size_t, size_t>;
using Buildings = std::vector<std::vector<int>>;


namespace {

enum Direction {
	kLeft = 0,
	kRight = 1,
	kTop = 2,
	kBottom = 3,
};

Direction Opposite(Direction k) {
	return Direction(k ^ 1);
}

struct Block {
	bool neighbor[4] = {};
	int neighbor_count = 0;
	int height = 0;
	int index = -1;
	int col = -1;
	int row = -1;
};

std::ostream& operator<<(std::ostream& stream, const Block& block) {
	stream << "[" << block.row << ", " << block.col << "]";
	stream << "(h=" << block.height << ", n=" << block.neighbor_count << ")";
	return stream;
}

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

		block.index = index;
		block.height = (h == 1 ? 5 : h); // rewrite rule
		block.row = row;
		block.col = col;

		// debug height
		if (h == 6) {
			block.height = 1;
		}

		if (h == 0) {
			return;
		}

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

	int NeighborIndex(int index, Direction dir) {
		auto mul = ((dir & 1) == 0 ? -1 : 1);
		auto diff = ((dir & 2) == 0 ? 1 : cols_);
		return mul * diff + index;
	}

	int NeighborHeight(int index, Direction dir) {
		return blocks_[NeighborIndex(index, dir)].height;
	}

	int CornerHeight(int index, Direction dir1, Direction dir2) {
		return NeighborHeight(NeighborIndex(index, dir1), dir2);
	}

	template<typename Function>
	void ForEachNeighbor(const Block& block, Function fn) {
		auto index = block.index;

		for (int k = 0; k < 4; ++k) {
			if (block.neighbor[k]) {
				fn(blocks_[NeighborIndex(index, Direction(k))], Direction(k));
			}
		}
	}

	void EliminateBlock(Block& block, bool is_newer) {
		ForEachNeighbor(block, [&](Block& nb, Direction dir) {
			if (is_newer) {
				assert(nb.height > 1);
				--nb.height;
			}
			--nb.neighbor_count;
			assert(nb.height == 5 || nb.height <= nb.neighbor_count + 1);
			nb.neighbor[Opposite(dir)] = false;
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

	bool IsTrapForNewer(const Block& block) {
		if (block.height != 5) {
			return false;
		}

		auto index = block.index;
		Direction vert[] = {kTop, kBottom};
		Direction horz[] = {kLeft, kRight};

		for (int v = 0; v < 2; ++v) {
			if (NeighborHeight(index, vert[v]) != 2) {
				continue;
			}

			for (int h = 0; h < 2; ++h) {
				if (NeighborHeight(index, horz[h]) == 2 &&
					CornerHeight(index, vert[v], horz[h]) == 2)
				{
					return true;
				}
			}
		}
		return false;
	}

	bool EliminateTrap() {
		// TODO: make this faster
		for (auto& block : blocks_) {
			if (IsTrapForNewer(block)) {
				EliminateBlock(block, false);
				return true;
			}
		}
		return false;
	}

	bool EliminateNext() {
		return EliminateNewer() || EliminateOlder() || EliminateTrap();
	}

	void Visual() {
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

	CommandVec GetCommands() {
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


// TODO:
//	- track true 5s (also for trap elimination)
//	- detect inconsistent state
//	- optimize command storage


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
	p.Visual();
	std::cout << "Steps: " << count << std::endl;
	solution = p.GetCommands();
}
