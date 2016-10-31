#include <vector>
#include <cassert>
#include <utility>
#include <iomanip>
#include <iostream>

using Buildings = std::vector<std::vector<int>>;
using Command = std::pair<size_t, size_t>;

std::ostream& operator<<(std::ostream& os, const Buildings& buildings) {
    auto h = buildings.size();
    auto w = buildings.front().size();

    os << h << " " << w << '\n';
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            os << buildings[y][x] << (x == w-1 ? "" : " ");
        }
        os << '\n';
    }
    return os;
}

void buildOuterLayer(int& building) {
    if (++building == 5) {
        building = 1;
    }
}

void applyCommand(Buildings& buildings, const Command& command) {
    auto h = buildings.size();
    auto w = buildings.front().size();

    auto y = command.first;
    auto x = command.second;

    assert(x < w && y < h);
    assert(buildings[y][x] == 0);

    ++buildings[y][x];

    if (x > 0)   { buildOuterLayer(buildings[y][x-1]); }
    if (y > 0)   { buildOuterLayer(buildings[y-1][x]); }
    if (x < w-1) { buildOuterLayer(buildings[y][x+1]); }
    if (y < h-1) { buildOuterLayer(buildings[y+1][x]); }
}

void CalculateBuildOrder(
    const Buildings& buildings,
    std::vector<Command>& commands)
{

}

Buildings applyBuildOrder(int h, int w, const std::vector<Command>& commands) {
    Buildings buildings(h, std::vector<int>(w, 0));

    for (auto command : commands) {
        applyCommand(buildings, command);
    }

    return buildings;
}

#ifdef LOCAL
int main() {
    int h, w;
    std::cin >> h >> w;

    Buildings buildings(h, std::vector<int>(w));

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            std::cin >> buildings[y][x];
        }
    }

    std::vector<Command> commands;
    CalculateBuildOrder(buildings, commands);

    Buildings resultBuildings = applyBuildOrder(h, w, commands);

    if (buildings == resultBuildings) {
        std::cout << "Match" << std::endl;
        return 0;
    } else {
        std::cout << "Mismatch :(" << std::endl;
        std::cout << "Commands:" << std::endl;
        for (auto command : commands) {
            std::cout << command.first << " " << command.second << '\n';
        }
        std::cout << "Input:" << std::endl;
        std::cout << buildings;
        std::cout << "Result:" << std::endl;
        std::cout << resultBuildings;
        return 1;
    }
}
#endif