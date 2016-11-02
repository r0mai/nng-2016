#include "calculate.h"
#include <vector>
#include <cassert>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <chrono>
#include <unordered_set>

#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/container/static_vector.hpp>
#include <boost/functional/hash.hpp>


namespace {

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

struct Point {
    Point() = default;
    Point(int y, int x) : y(y), x(x) {}
    int y = 0;
    int x = 0;
};


class Parcel {
public:
    Parcel(const Buildings& buildings) :
        buildings(buildings),
        h(buildings.size()),
        w(buildings.front().size())
    {
        rewrite();
    }

    const Buildings& getBuildings() const { return buildings; }
    const std::vector<Command>& getHistory() const { return history; }

    void rewrite() {
        for (int y = 1; y < h-1; ++y) {
            for (int x = 1; x < w-1; ++x) {
                if (buildings[y][x] == 1) { buildings[y][x] = 5; }
            }
        }
    }

    boost::container::static_vector<Point, 4> getNeighbors(const Point& p) {
        boost::container::static_vector<Point, 4> result;

        if (p.x > 0)   { result.push_back({p.y, p.x-1}); }
        if (p.y > 0)   { result.push_back({p.y-1, p.x}); }
        if (p.x < w-1) { result.push_back({p.y, p.x+1}); }
        if (p.y < h-1) { result.push_back({p.y+1, p.x}); }

        return result;
    }

    void unbuild(const Point& p) {
        assert(buildings[p.y][p.x] == 1);
        buildings[p.y][p.x] = 0;
        history.push_back({p.y, p.x});

        for (const Point& n : getNeighbors(p)) {
            int& height = buildings[n.y][n.x];
            assert(height == 0 || height > 1);
            height = std::max(height - 1, 0);
        }
    }

    boost::optional<Point> FindHeight(int height) {
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (buildings[y][x] == height) {
                    return Point{y, x};
                }
            }
        }
        return boost::none;
    }

    bool demolish() {
        Point p;
        auto p1 = FindHeight(1);
        if (!p1) {
            auto p5 = FindHeight(5);
            if (!p5) {
                return false;
            }
            p = *p5;
            buildings[p.y][p.x] = 1;
        } else {
            p = *p1;
        }
        unbuild(p);
        return true;
    }

private:
    Buildings buildings;
    std::vector<Command> history;
    int w;
    int h;
};


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

int getZeroNeighbourCount(int h, int w, int y, int x, const Buildings& buildings) {
    int count = 0;
    count += (x > 0 && buildings[y][x-1] == 0);
    count += (y > 0 && buildings[y-1][x] == 0);
    count += (x < w-1 && buildings[y][x+1] == 0);
    count += (y < h-1 && buildings[y+1][x] == 0);
    return count;
}

struct BuildingsHash {
    size_t operator()(const Buildings& buildings) const {
        size_t hash = 0;

        auto h = buildings.size();
        auto w = buildings.front().size();

        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                boost::hash_combine(hash, buildings[y][x]);
            }
        }

        return hash;
    }
};

std::unordered_set<Buildings, BuildingsHash> buildingsHistory;

bool backtrackRecurse(int h, int w, Buildings& buildings, std::vector<Command>& commands, int depth) {
    if (!buildingsHistory.insert(buildings).second) {
        return false;
    }

    bool has_non_zero = false;

    std::vector<std::pair<Command, int>> candidates;
    candidates.reserve(w * h);

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            switch (buildings[y][x]) {
                default:
                    has_non_zero = true;
                    break;
                case 1: {
                    has_non_zero = true;
                    Command cmd = {y, x};
                    candidates.push_back({cmd, getZeroNeighbourCount(h, w, y, x, buildings)});
                    break;
                }
                case 0:
                    break;
            }
        }
    }
    if (!has_non_zero) {
        return true;
    }

    std::sort(begin(candidates), end(candidates),
        [](const std::pair<Command, int>& lhs, const std::pair<Command, int>& rhs) {
            int l = lhs.second;
            int r = rhs.second;
            if (l == 0) { l = 20; }
            if (r == 0) { r = 20; }
            return l < r;
        }
    );

    for (const auto& candidate : candidates) {
        auto& cmd = candidate.first;
        commands.push_back(cmd);
        applyCommandInverse(buildings, cmd);
        if (backtrackRecurse(h, w, buildings, commands, depth+1)) {
            return true;
        }
        applyCommand(buildings, cmd);
        commands.pop_back();
    }

    return false;
}

} // namespace


void CalculateBuildOrder(
    const Buildings& buildings,
    std::vector<Command>& commands)
{
    commands.clear();
#if 1
    Parcel parcel(buildings);
    while (parcel.demolish()) {
        std::cout << parcel.getBuildings() << std::endl;
    }

    commands = parcel.getHistory();
    std::reverse(begin(commands), end(commands));
#else
    auto h = buildings.size();
    auto w = buildings.front().size();

    commands.reserve(w * h);

    auto copy = buildings;
    backtrackRecurse(h, w, copy, commands, 1);
    std::reverse(begin(commands), end(commands));
#endif
}
