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

enum Color {
	kClear,
	kRed,
	kGreen
	// should fit into 2 bits
};

enum Direction {
	kLeft = 0,
	kRight = 1,
	kTop = 2,
	kBottom = 3
};

enum Mode {
	kExact,
	kGuess
};

inline Direction Opposite(Direction k) {
	return Direction(k ^ 1);
}

struct Block {
	Block() {
		neighbor = 0;
		in_stack = 0;
		color = kClear;
	}

	int index = -1;
	uint8_t neighbor_count = 0;
	uint8_t height = 0;
	uint8_t last_height = 0;
	uint8_t neighbor : 4;
	uint8_t in_stack : 1;
	uint8_t color : 2;
};

struct Item {
	Item(int index, bool is_newer = false, bool mark = false)
		: index(index)
		, is_newer(is_newer)
		, mark(mark)
	{}

	unsigned index : 24;
	unsigned is_newer : 1;
	unsigned mark : 1;
};

struct Position {
	int row;
	int col;
};

struct Range {
	int range_begin;
	int range_end;
};


std::ostream& operator<<(std::ostream& ss, const Position& pos) {
	ss << (pos.row + 1) << " " << (pos.col + 1);
	return ss;
}

std::ostream& operator<<(std::ostream& ss, const Block& block) {
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
using RangeVec = std::vector<Range>;


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


class Partition {
public:
	struct Iterator {
		int index;
		RangeVec::iterator it;
		RangeVec::iterator it_end;

		bool operator==(const Iterator& it) const {
			return index == it.index;
		}

		bool operator!=(const Iterator& it) const {
			return index != it.index;
		}

		int operator*() const {
			return index;
		}

		Iterator operator++(int) {
			Iterator copy = *this;
			++copy;
			return copy;
		}

		Iterator& operator++() {
			if (index != -1) {
				if (index + 1 < it->range_end) {
					++index;
				} else {
					++it;
					if (it == it_end) {
						index = -1;
					} else {
						index = it->range_begin;
					}
				}
			}
			return *this;
		}
	};

	Iterator begin() {
		if (ranges_.empty()) {
			return end();
		}
		return {ranges_.front().range_begin, ranges_.begin(), ranges_.end()};
	}

	Iterator end() {
		return {-1, ranges_.end(), ranges_.end()};
	}

	void Insert(const Range& range) {
		assert(range.range_begin < range.range_end);
		ranges_.push_back(range);
		size_ += range.range_end - range.range_begin;
	}

	void Insert(int range_begin, int range_end) {
		if (range_begin < range_end) {
			ranges_.push_back({range_begin, range_end});
			size_ += range_end - range_begin;
		} else {
			assert(false);
		}
	}

	int Size() const {
		return size_;
	}

	void SetEye(int index) {
		eye_ = index;
	}

	int Eye() const {
		return eye_;
	}

private:
	int eye_ = -1;
	int size_ = 0;
	RangeVec ranges_;
};


using PartitionVec = std::vector<Partition>;


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
				left.neighbor |= (1<<kRight);
				block.neighbor |= (1<<kLeft);
				++left.neighbor_count;
				++block.neighbor_count;
			}
		}

		if (row > 0) {
			auto& top = blocks_[NeighborIndex(index, kTop)];
			if (top.height) {
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

		Partition full_area;
		full_area.Insert(0, rows_ * cols_);
		areas_.push_back(std::move(full_area));

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

	Position GetPosition(int index) {
		assert(cols_ > 0);
		auto d = div(index, cols_);
		return {d.quot, d.rem};
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

	Partition ColorizeDfs(Block& block) {
		// Returns
		//	size	- size of the area
		//	index	- index of a block with h=5, or -1
		//	ccount	- candidate count
		//
		BlockPtrVec& vec = block_ptrs_;
		Partition result;
		vec.clear();

		int row_max = 0;
		int col_max = 0;
		int row_min = rows_;
		int col_min = cols_;

		block.color = kGreen;
		vec.push_back(&block);

		while (!vec.empty()) {
			auto& b = *vec.back();
			vec.pop_back();

			if (b.height == 5) {
				result.SetEye(b.index);
			}

			auto d = div(b.index, cols_);
			int row = d.quot;
			int col = d.rem;

			row_min = std::min(row_min, row);
			row_max = std::max(row_max, row + 1);
			col_min = std::min(col_min, col);
			col_max = std::max(col_max, col + 1);

			for (int i = 0; i < 4; ++i) {
				if ((b.neighbor & (1<<i))) {
					auto& nb = blocks_[NeighborIndex(b.index, Direction(i))];
					if (nb.color != kClear) {
						continue;
					}
					nb.color = kGreen;
					vec.push_back(&nb);
				}
			}
		}

		for (int row = row_min; row < row_max; ++row) {
			Range current {-1, -1};
			bool in_range = false;
			int index = row * cols_ + col_min;
			for (int i = col_max - col_min; i-- > 0; ++index) {
				auto& b = blocks_[index];
				if (!in_range && b.color == kGreen) {
					in_range = true;
					current.range_begin = index;
				} else if (in_range && b.color != kGreen) {
					in_range = false;
					current.range_end = index;
					result.Insert(current);
				}
			}
			if (in_range) {
				current.range_end = index;
				result.Insert(current);
			}
		}

		for (auto i : result) {
			blocks_[i].color = kRed;
		}

		assert(result.Size() > 0);

		return result;
	}

	struct ColorCheckResult {
		bool valid = true;
		PartitionVec areas;
	};

	ColorCheckResult ColorCheck(bool collect_partitions) {
		// Returns
		//	valid	- true, if the map is solvable
		//	size	- size of the best same-colored area
		//	index	- index of a h=5 block in the best area
		//
		++checks_;

		int best = -1;
		ColorCheckResult result;
		int isolated = 0;
		auto& current_area = areas_.back();

		for (auto index : current_area) {
			auto& b = blocks_[index];
			b.color = kClear;
		}

		for (auto index : current_area) {
			auto& b = blocks_[index];
			if (b.height == 0 || b.color != kClear) {
				continue;
			}

			auto cc = ColorizeDfs(b);
			if (cc.Eye() == -1) {
				result.valid = false;
				break;
			}

			if (collect_partitions) {
				result.areas.push_back(std::move(cc));
			} else if (best == -1 || cc.Size() < best) {
				best = cc.Size();
				result.areas.clear();
				result.areas.push_back(std::move(cc));
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

		auto exact = (mode_ == kExact);
		auto cc = ColorCheck(exact);

		if (!cc.valid) {
			// std::cerr << "- Invalid state" << std::endl;
			return Rollback();
		}

		if (exact) {
			std::move(cc.areas.begin(), cc.areas.end(),
				std::back_inserter(areas_));
			mode_ = kGuess;
			return true;
		}

		if (!cc.areas.empty()) {
			auto& next_area = cc.areas.back();
			auto& nb = blocks_[next_area.Eye()];
			bool is_newer = true;

			// Leave a mark behind in the history,
			// so backtrack stops here
			history_.push_back({nb.index, !is_newer, true});
			++marks_;

			// std::cerr << "Area " << next_area.Size() << std::endl;
			// std::cerr << "Guess #" << marks_ << " " << nb << std::endl;
			// Visual();
			Push(nb, is_newer);
			return true;
		}

		areas_.pop_back();
		if (areas_.empty()) {
			return false;
		}

		return true;
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
			// Visual();
			std::cerr << "Invalid history ";
			std::cerr << older << " != " << newer + 1 << std::endl;
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

	Mode mode_ = kExact;

	BlockVec blocks_;
	BlockPtrVec block_ptrs_;
	ItemVec stack_;
	ItemVec history_;
	PartitionVec areas_;
};

} // namespace




void CalculateBuildOrder(const Buildings& buildings,
	std::vector<Command>& solution)
{
	// std::cerr << sizeof(Partition) << std::endl;
	// std::cerr << sizeof(Block) << std::endl;
	Parcel p;
	p.Init(buildings);

	// p.Visual();
	while (p.EliminateNext()) {
		// loop
	}
	// p.Visual();
	// p.Stats();
	solution = p.GetCommands();
}
