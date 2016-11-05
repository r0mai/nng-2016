// do not include any local headers here
#include <iostream>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <utility>
#include <cassert>
#include <cstddef>
#include <cstdlib>


using Command = std::pair<size_t, size_t>;
using Buildings = std::vector<std::vector<int>>;


namespace {

enum Direction {
	kLeft = 0,
	kRight = 1,
	kTop = 2,
	kBottom = 3,
};

inline Direction Opposite(Direction k) {
	return Direction(k ^ 1);
}

struct Block {
	bool neighbor[4] = {};
	int index = -1;
	int neighbor_count = 0;
	int height = 0;
	int last_height = 0;
	int row = -1;
	int col = -1;
	bool in_stack = false;
};

struct Item {
	Item(int index, bool is_newer = false, bool mark = false)
		: index(index)
		, is_newer(is_newer)
		, mark(mark) {
	}

	unsigned index : 24;
	unsigned is_newer : 1;
	unsigned mark : 1;
};


std::ostream& operator<<(std::ostream& ss, const Block& block) {
	ss << block.row + 1 << " " << block.col + 1;
	ss << " (h=" << block.height << ", n=" << block.neighbor_count << ")";
	return ss;
}


bool IsOlder(const Block& block) {
	return (block.height > 1 && block.height < 5 &&
		block.height >= block.neighbor_count + 1);
}

bool IsNewer(const Block& block) {
	return (block.height == 1 ||
		(block.height == 5 && block.neighbor_count < 4));
}

using BlockVec = std::vector<Block>;
using IntVec = std::vector<int>;
using CommandVec = std::vector<Command>;
using IntSet = std::unordered_set<int>;
using ItemVec = std::vector<Item>;

class Parcel {
public:
	void Init(const Buildings& bs) {
		size_ = 0;
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
		block.neighbor_count = 0;
		block.row = row;
		block.col = col;

		// debug height
		if (h == 6) {
			block.height = 1;
		}

		if (h == 0) {
			return;
		}

		++size_;
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
		stack_.reserve(rows_ * cols_);
		history_.reserve(rows_ * cols_);

		for (auto& block : blocks_) {
			if (IsOlder(block)) {
				Push(block);
			} else if (IsNewer(block)) {
				Push(block, true);
			}
		}

		std::cerr << "# " << stack_.size() << " ";
		std::cerr << history_.size() << std::endl;
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

	void Push(Block& block, bool is_newer = false) {
		if (!block.in_stack) {
			// std::cerr << "> " << is_newer << " " << block.index << std::endl;
			block.in_stack = true;
			stack_.push_back({block.index, is_newer});
		}
	}

	Block& Pop(bool& is_newer) {
		auto item = stack_.back();
		is_newer = item.is_newer;
		auto& block = blocks_[item.index];
		block.in_stack = false;
		// std::cerr << "< " << is_newer << " " << block.index << std::endl;

		stack_.pop_back();
		history_.push_back(item);
		return block;
	}

	bool EliminateBlock(Block& block, bool is_newer) {
		auto index = block.index;
		bool invalid = false;

		if ((is_newer && (block.height & 3) != 1) ||
			(!is_newer && block.height > block.neighbor_count + 1))
		{
			std::cerr << "- Invalid" << std::endl;
			return false;
		}

		for (int k = 0; k < 4; ++k) {
			if (block.neighbor[k]) {
				auto dir = Direction(k);
				auto& nb = blocks_[NeighborIndex(index, dir)];
				// std::cerr << "? " << is_newer << " " << nb << std::endl;
				if ((is_newer && nb.height == 1) ||
					(nb.height < 5 && nb.height > nb.neighbor_count + is_newer))
				{
					std::cerr << "? " << is_newer << " " << nb << std::endl;
					invalid = true;
					break;
				}
			}
		}

		if (invalid) {
			return false;
		}

		for (int k = 0; k < 4; ++k) {
			if (block.neighbor[k]) {
				auto dir = Direction(k);
				auto& nb = blocks_[NeighborIndex(index, dir)];
				if (is_newer) {
					--nb.height;
				}
				--nb.neighbor_count;
				nb.neighbor[Opposite(dir)] = false;
				if (IsOlder(nb)) {
					Push(nb);
				} else if (IsNewer(nb)) {
					Push(nb, true);
				}
			}
		}

		block.last_height = block.height;
		block.height = 0;
		return true;
	}

	void RestoreBlock(Block& block, bool is_newer) {
		auto index = block.index;
		for (int k = 0; k < 4; ++k) {
			if (block.neighbor[k]) {
				auto dir = Direction(k);
				auto& nb = blocks_[NeighborIndex(index, dir)];
				if (is_newer) {
					++nb.height;
				}
				++nb.neighbor_count;
				nb.neighbor[Opposite(dir)] = true;
			}
		}
		block.height = block.last_height;
	}

	Block* FindNext() {
		for (auto& block : blocks_) {
			if (block.height == 5) {
				return &block;
			}
		}
		return nullptr;
	}

	void ClearStack() {
		for (auto item : stack_) {
			blocks_[item.index].in_stack = false;
		}
		stack_.clear();
	}

	bool Rollback() {
		ClearStack();
		while (!history_.empty()) {
			auto item = history_.back();
			auto& block = blocks_[item.index];
			history_.pop_back();

			if (item.mark) {
				--marks_;
				Push(block, item.is_newer);
				std::cerr << "Retry " << block << std::endl;
				return true;
			}
			if (block.height == 0) {
				RestoreBlock(block, item.is_newer);
			}
		}

		std::cerr << "No luck" << std::endl;
		return false;
	}

	bool Sanity() {
		int sum = 0;
		int rank2 = 0;
		for (auto& b : blocks_) {
			if (b.height == 0) { continue; }
			sum += b.height;
			rank2 += b.neighbor_count + 2;
		}
		bool sane = abs(sum - rank2 / 2) % 4 == 0;
		if (!sane) {
			std::cerr << "SANITY CHECK FAILED" << std::endl;
		}
		return sane;
	}

	bool EliminateNext() {
		++steps_;
		while (!stack_.empty()) {
			bool is_newer = false;
			auto& block = Pop(is_newer);

			if (block.height > 0) {
				bool success = EliminateBlock(block, is_newer);
				if (!success) {
					std::cerr << "- Failed " << block;
					std::cerr << "  Hsize=" << history_.size() << std::endl;
					// Visual();
					return Rollback();
				}
				return true;
			}
		}

		auto next = FindNext();
		if (next) {
			// Leave a mark behind in the history,
			// so backtrack stops here
			++marks_;
			history_.push_back({next->index, true, true});
			std::cerr << "Guess " << *next << std::endl;
			// Visual();
			Push(*next);
			return true;
		}

		if (history_.size() - marks_ != size_) {
			std::cerr << "Invalid state" << std::endl;
			return Rollback();
		}

		return false;
	}

	void Visual() {
		// can has stdio?
		bool show_height = false;

		show_height = true;
		if (show_height) {
			std::cout << rows_ << " " << cols_ << std::endl;

			for (const auto& block : blocks_) {
				int row = block.row;
				int col = block.col;
				if (col > 0) {
					std::cout << " ";
				} else if (row > 0) {
					std::cout << std::endl;
				}
				std::cout << block.height;
			}
			std::cout << std::endl;
		}
	}

	void Stats() {
		std::cerr << "Steps: " << steps_ << std::endl;
		std::cerr << "History: " << history_.size() << " - " << marks_ << std::endl;
	}

	CommandVec GetCommands() {
		CommandVec vec;
		int older = 0;
		int newer = size_ - 1;

		vec.resize(size_);
		for (auto item : history_) {
			if (item.mark) {
				continue;
			}

			auto d = div(item.index, cols_);
			int row = d.quot;
			int col = d.rem;

			assert(older <= newer);
			if (item.is_newer) {
				vec[newer--] = {row, col};
			} else {
				vec[older++] = {row, col};
			}
		}
		if (older != newer + 1) {
			std::cerr << "Invalid history ";
			std::cerr << older << " + " << (size_ - 1 - newer) << std::endl;
			assert(false);
		}
		return vec;
	}

private:
	int rows_ = 0;
	int cols_ = 0;
	int steps_ = 0;
	int size_ = 0;
	int marks_ = 0;

	BlockVec blocks_;
	ItemVec stack_;
	ItemVec history_;
};

} // namespace


void CalculateBuildOrder(const Buildings& buildings,
	std::vector<Command>& solution)
{
	Parcel p;
	p.Init(buildings);

	// p.Visual();
	while (p.EliminateNext()) {
		if (!p.Sanity()) {
			break;
		}
	}
	// p.Visual();

	p.Stats();
	solution = p.GetCommands();
}
