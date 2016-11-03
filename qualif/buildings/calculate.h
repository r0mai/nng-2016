#include <vector>
#include <utility>
#include <cstddef>

using Command = std::pair<size_t, size_t>;
using Buildings = std::vector<std::vector<int>>;

void CalculateBuildOrder(const std::vector<std::vector<int>>& buildings,
	std::vector<std::pair<size_t, size_t>>& solution);
