#include <iostream>
#include <vector>
#include <cassert>

using Buildings = std::vector<std::vector<int>>;
using Command = std::pair<int, int>;
using Commands = std::vector<Command>;

Buildings createEmpty(int rows, int cols) {
	return Buildings(rows, std::vector<int>(cols));
}

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

Buildings applyCommands(int rows, int cols, const std::vector<Command>& commands) {
	auto buildings = createEmpty(rows, cols);

    for (auto command : commands) {
        applyCommand(buildings, command);
    }

    return buildings;
}

Commands rowMajor(int rows, int cols) {
	Commands commands;
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			commands.push_back({y, x});
		}
	}
	return commands;
}

Commands checkerBoard(int rows, int cols) {
	Commands commands;
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			if ((x + y) % 2 == 0) {
				commands.push_back({y, x});
			}
		}
	}
	for (int y = 0; y < rows; ++y) {
		for (int x = 0; x < cols; ++x) {
			if ((x + y) % 2 == 1) {
				commands.push_back({y, x});
			}
		}
	}
	return commands;
}

Commands randomMap(int rows, int cols) {
    std::vector<Command> commands;
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            commands.push_back({y, x});
        }
    }
    std::random_shuffle(begin(commands), end(commands));

	return commands;
}

int main(int argc, char **argv) {
	if (argc != 4 && argc != 5) {
		std::cerr << "Usage ./generate <type> <rows> <cols> [<seed>]" << std::endl;
		return 1;
	}

	std::string type = argv[1];
	int rows = std::stoi(argv[2]);
	int cols = std::stoi(argv[3]);
	int seed = std::time(nullptr);
	if (argc == 5) {
		seed = std::stoi(argv[4]);
	}

	std::cerr << "Seed is " << seed << std::endl;
	std::srand(seed);

	Commands commands;
	if (type == "rowmajor") {
		commands = rowMajor(rows, cols);
	} else if (type == "checker") {
		commands = checkerBoard(rows, cols);
	} else if (type == "random") {
		commands = randomMap(rows, cols);
	} else {
		std::cerr << "Unknown type" << std::endl;
		return 1;
	}

	std::cout << applyCommands(rows, cols, commands) << std::endl;
}
