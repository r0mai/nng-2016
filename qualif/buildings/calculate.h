#include <vector>
#include <utility>

using Buildings = std::vector<std::vector<int>>;
using Command = std::pair<size_t, size_t>;

void CalculateBuildOrder(const std::vector<std::vector<int>>& buildings,
	std::vector<std::pair<size_t, size_t>>& solution);
