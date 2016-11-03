#include <vector>
#include <utility>

using Command = std::pair<size_t, size_t>;
using Buildings = std::vector<std::vector<int>>;

void CalculateBuildOrder(const std::vector<std::vector<int>>& buildings,
	std::vector<std::pair<size_t, size_t>>& solution);
