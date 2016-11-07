// do not include any local headers here
#include <iostream>
#include <algorithm>
#include <vector>
#include <utility>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include <cstring>


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
	Block() {
		neighbor = 0;
		in_stack = 0;
	}

	int index = -1;
	uint8_t neighbor_count = 0;
	uint8_t height = 0;
	uint8_t last_height = 0;
	uint8_t neighbor : 4;
	uint8_t in_stack : 1;
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
	// ss << block.row + 1 << " " << block.col + 1;
	ss << " (h=" << int(block.height) << ", n=";
	ss << int(block.neighbor_count) << ")";
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
using BlockPtrVec = std::vector<Block*>;
using IntVec = std::vector<int>;
using CommandVec = std::vector<Command>;
using ItemVec = std::vector<Item>;


class BitArray {
public:
	BitArray() = default;
	~BitArray() {
		if (array_) {
			delete[] array_;
		}
	}

	void Init(int size) {
		assert(!array_);

		size_ = size;
		array_ = new uint8_t[size];
	}

	void Clear() {
		memset(array_, 0, size_);
	}

	bool IsSet(int index) const {
		return (array_[index >> 3] & (1 << (index & 7)));
	}

	void Set(int index) {
		assert(index >= 0 && index < size_);
		array_[index >> 3] |= (1 << (index & 7));
	}

private:
	BitArray(const BitArray&) = delete;
	BitArray& operator=(const BitArray&) = delete;

	int size_ = 0;
	uint8_t* array_ = nullptr;
};


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
				// left.neighbor[kRight] = true;
				// block.neighbor[kLeft] = true;
				left.neighbor |= (1<<kRight);
				block.neighbor |= (1<<kLeft);
				++left.neighbor_count;
				++block.neighbor_count;
			}
		}

		if (row > 0) {
			auto& top = blocks_[NeighborIndex(index, kTop)];
			if (top.height) {
				// top.neighbor[kBottom] = true;
				// block.neighbor[kTop] = true;
				top.neighbor |= (1<<kBottom);
				block.neighbor |= (1<<kTop);
				++top.neighbor_count;
				++block.neighbor_count;
			}
		}
	}

	void InitStacks() {
		stack_.reserve(rows_ * cols_);
		history_.reserve(rows_ * cols_);
		block_ptrs_.reserve(rows_ * cols_);
		colors_.Init(rows_ * cols_);

		for (auto& block : blocks_) {
			if (IsOlder(block)) {
				Push(block);
			} else if (IsNewer(block)) {
				Push(block, true);
			}
		}

		// std::cerr << "# " << stack_.size() << " ";
		// std::cerr << history_.size() << std::endl;
	}

	int BlockIndex(int row, int col) {
		return col + row * cols_;
	}

	inline int NeighborIndex(int index, Direction dir) {
		auto mul = ((dir & 1) == 0 ? -1 : 1);
		auto diff = ((dir & 2) == 0 ? 1 : cols_);
		return mul * diff + index;
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
			// std::cerr << "- Invalid" << std::endl;
			return false;
		}

		for (int k = 0; k < 4; ++k) {
			if ((block.neighbor & (1<<k))) {
				auto dir = Direction(k);
				auto& nb = blocks_[NeighborIndex(index, dir)];
				// std::cerr << "? " << is_newer << " " << nb << std::endl;
				if ((is_newer && nb.height == 1) ||
					(nb.height < 5 && nb.height > nb.neighbor_count + is_newer))
				{
					// std::cerr << "? " << is_newer << " " << nb << std::endl;
					invalid = true;
					break;
				}
			}
		}

		if (invalid) {
			return false;
		}

		for (int k = 0; k < 4; ++k) {
			if ((block.neighbor & (1<<k))) {
				auto dir = Direction(k);
				auto& nb = blocks_[NeighborIndex(index, dir)];
				if (is_newer) {
					--nb.height;
				}
				--nb.neighbor_count;
				nb.neighbor &= ~(1<<Opposite(dir));
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
			if ((block.neighbor & (1<<k))) {
				auto dir = Direction(k);
				auto& nb = blocks_[NeighborIndex(index, dir)];
				if (is_newer) {
					++nb.height;
				}
				++nb.neighbor_count;
				nb.neighbor |= (1<<Opposite(dir));
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
				// std::cerr << "- Retry " << block << std::endl;
				return true;
			}
			if (block.height == 0) {
				RestoreBlock(block, item.is_newer);
			}
		}

		// std::cerr << "No luck" << std::endl;
		return false;
	}

	struct ColorizeResult {
		int size = 0;
		int index = -1;
		int ccount = 0;
	};

	ColorizeResult ColorizeDfs(Block& block) {
		// Returns
		//	size	- size of the area
		//	index	- index of a block with h=5, or -1
		//	ccount	- candidate count
		//
		BlockPtrVec& vec = block_ptrs_;
		ColorizeResult result;
		vec.clear();

		colors_.Set(block.index);
		vec.push_back(&block);

		while (!vec.empty()) {
			auto& b = *vec.back();
			vec.pop_back();

			++result.size;

			if (b.height == 5) {
				result.index = b.index;
				++result.ccount;
			}

			for (int i = 0; i < 4; ++i) {
				if ((b.neighbor & (1<<i))) {
					auto& nb = blocks_[NeighborIndex(b.index, Direction(i))];
					if (colors_.IsSet(nb.index)) {
						continue;
					}
					colors_.Set(nb.index);
					vec.push_back(&nb);
				}
			}
		}

		return result;
	}

	struct Best {
		bool empty = true;
		int value = 0;
	};

	bool IsBetter(const ColorizeResult& cc, Best& best) {
		// int value = cc.ccount;
		int value = 0;
		if (best.empty || value < best.value) {
			best.empty = false;
			best.value = value;
			return true;
		}
		return false;
	}

	struct ColorCheckResult {
		bool valid = true;
		int size = 0;
		int index = -1;
		int ccount = 0;
	};

	ColorCheckResult ColorCheck() {
		// Returns
		//	valid	- true, if the map is solvable
		//	size	- size of the best same-colored area
		//	index	- index of a h=5 block in the best area
		//
		++checks_;

		Best best;
		ColorCheckResult result;
		int k = 0;

		colors_.Clear();
		for (auto& b : blocks_) {
			if (b.height == 0 || colors_.IsSet(b.index)) {
				continue;
			}

			auto cc = ColorizeDfs(b);
			if (cc.index == -1) {
				result.valid = false;
				break;
			}
			if (cc.index != -1 && IsBetter(cc, best)) {
				result.size = cc.size;
				result.index = cc.index;
				result.ccount = cc.ccount;
			}
		}
		return result;
	}

	bool EliminateNext() {
		++steps_;
		while (!stack_.empty()) {
			bool is_newer = false;
			auto& block = Pop(is_newer);

			if (block.height > 0) {
				bool success = EliminateBlock(block, is_newer);
				if (!success) {
					// std::cerr << "- Failed " << block;
					// std::cerr << "  Hsize=" << history_.size() << std::endl;
					// Visual();
					return Rollback();
				}
				return true;
			}
		}

		auto cc = ColorCheck();
		// cc = ColorCheck();
		if (!cc.valid) {
			// std::cerr << "- Invalid state" << std::endl;
			return Rollback();
		}

		if (cc.index != -1) {
			auto& nb = blocks_[cc.index];
			bool is_newer = true;

			// Leave a mark behind in the history,
			// so backtrack stops here
			++marks_;
			history_.push_back({nb.index, !is_newer, true});
			// std::cerr << "Area " << cc.size << std::endl;
			// std::cerr << "Guess #" << marks_ << " " << nb;
			// std::cerr << " ccount=" << cc.ccount << std::endl;
			// Visual();
			Push(nb, is_newer);
			return true;
		}

		if (history_.size() - marks_ != size_) {
			std::cerr << marks_ << " Invalid state" << std::endl;
			// Visual();
			// return false;
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
				auto d = div(block.index, cols_);
				int row = d.quot;
				int col = d.rem;
				if (col > 0) {
					std::cout << " ";
				} else if (row > 0) {
					std::cout << std::endl;
				}
				std::cout << int(block.height);
			}
			std::cout << std::endl;
		}
	}

	void Stats() {
		std::cerr << "--" << std::endl;
		std::cerr << "Steps:  " << steps_ << std::endl;
		std::cerr << "Checks: " << checks_ << std::endl;
		std::cerr << std::endl;
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
	int checks_ = 0;

	BlockVec blocks_;
	BlockPtrVec block_ptrs_;
	BitArray colors_;
	ItemVec stack_;
	ItemVec history_;
};

} // namespace


void CalculateBuildOrder(const Buildings& buildings,
	std::vector<Command>& solution)
{
	// std::cerr << sizeof(Block) << std::endl;

	Parcel p;
	p.Init(buildings);
	// return;

	// p.Visual();
	while (p.EliminateNext()) {
		// loop
	}
	// p.Visual();
	// p.Stats();
	solution = p.GetCommands();
}

