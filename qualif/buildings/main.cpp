#include <vector>
#include <cassert>
#include <utility>
#include <iomanip>
#include <iostream>
#include <fstream>

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
    if (building != 0 && ++building == 5) {
        building = 1;
    }
}

void destructOuterLayer(int& building) {
    if (building == 1) {
        building = 4;
    } else if (building != 0) {
        --building;
    }
}

void applyCommand(Buildings& buildings, const Command& command) {
    auto h = buildings.size();
    auto w = buildings.front().size();

    auto y = command.first;
    auto x = command.second;

    assert(x < w && y < h);
    assert(buildings[y][x] == 0);

    buildings[y][x] = 1;

    if (x > 0)   { buildOuterLayer(buildings[y][x-1]); }
    if (y > 0)   { buildOuterLayer(buildings[y-1][x]); }
    if (x < w-1) { buildOuterLayer(buildings[y][x+1]); }
    if (y < h-1) { buildOuterLayer(buildings[y+1][x]); }
}

void applyCommandInverse(Buildings& buildings, const Command& command) {
    auto h = buildings.size();
    auto w = buildings.front().size();

    auto y = command.first;
    auto x = command.second;

    assert(x < w && y < h);
    assert(buildings[y][x] == 1);

    buildings[y][x] = 0;

    if (x > 0)   { destructOuterLayer(buildings[y][x-1]); }
    if (y > 0)   { destructOuterLayer(buildings[y-1][x]); }
    if (x < w-1) { destructOuterLayer(buildings[y][x+1]); }
    if (y < h-1) { destructOuterLayer(buildings[y+1][x]); }
}

bool backtrackRecurse(int h, int w, const Buildings& buildings, std::vector<Command>& commands) {
    std::cout << "Current:" << std::endl;
    std::cout << buildings << std::endl;
    bool has_non_zero = false;
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            switch (buildings[y][x]) {
                default:
                    has_non_zero = true;
                    break;
                case 1: {
                    has_non_zero = true;
                    commands.push_back({y, x});
                    auto copy = buildings;
                    applyCommandInverse(copy, commands.back());
                    if (backtrackRecurse(h, w, copy, commands)) {
                        return true;
                    }
                    commands.pop_back();
                    break;
                }
                case 0:
                    break;
            }
        }
    }
    return !has_non_zero;
}

void CalculateBuildOrder(
    const Buildings& buildings,
    std::vector<Command>& commands)
{
    commands.clear();

    auto h = buildings.size();
    auto w = buildings.front().size();

    backtrackRecurse(h, w, buildings, commands);
    std::reverse(begin(commands), end(commands));
}

Buildings applyBuildOrder(int h, int w, const std::vector<Command>& commands) {
    Buildings buildings(h, std::vector<int>(w, 0));

    for (auto command : commands) {
        applyCommand(buildings, command);
    }

    return buildings;
}

#ifdef LOCAL
int main(int argc, char** argv) {
    std::ifstream in_file;
    std::istream& in = argc == 2 ? in_file : std::cin;
    if (argc == 2) {
        in_file.open(argv[1]);
    }
    int h, w;
    in >> h >> w;

    Buildings buildings(h, std::vector<int>(w));

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            in >> buildings[y][x];
        }
    }

    std::vector<Command> commands;
    CalculateBuildOrder(buildings, commands);

    Buildings resultBuildings = applyBuildOrder(h, w, commands);

    if (buildings == resultBuildings) {
        std::cout << "Match" << std::endl;
        std::cout << "Commands:" << std::endl;
        for (auto command : commands) {
            std::cout << command.first << " " << command.second << '\n';
        }
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
