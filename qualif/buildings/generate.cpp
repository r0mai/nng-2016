#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <ctime>

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

Commands snail(int rows, int cols) {
	Commands commands;
	int begin_y = 0;
	int begin_x = 0;
	int end_y = rows;
	int end_x = cols;
	while (commands.size() < rows * cols) {
		// 1
		for (int x = begin_x; x < end_x; ++x) {
			commands.push_back({begin_y, x});
		}
		// 2
		for (int y = begin_y+1; y < end_y; ++y) {
			commands.push_back({y, end_x - 1});
		}
		// 3
		for (int x = end_x - 2; x >= begin_x; --x) {
			commands.push_back({end_y - 1, x});
		}
		// 4
		for (int y = end_y - 2; y >= begin_y + 1; --y) {
			commands.push_back({y, begin_x});
		}
		++begin_y;
		++begin_x;
		--end_x;
		--end_y;
	}
	return commands;
}

Commands randomBox3(int rows, int cols, std::mt19937& rng) {
	Commands commands;
    for (int y = 0; y < rows; y += 3) {
        for (int x = 0; x < cols; x += 3) {
			Commands subCommands;
			int r, c;
			r = y  ; c = x  ; if (r < rows && c < cols) { subCommands.push_back({r, c}); }
			r = y+1; c = x  ; if (r < rows && c < cols) { subCommands.push_back({r, c}); }
			r = y+2; c = x  ; if (r < rows && c < cols) { subCommands.push_back({r, c}); }
			r = y+2; c = x+1; if (r < rows && c < cols) { subCommands.push_back({r, c}); }
			r = y+2; c = x+2; if (r < rows && c < cols) { subCommands.push_back({r, c}); }
			r = y+1; c = x+2; if (r < rows && c < cols) { subCommands.push_back({r, c}); }
			r = y  ; c = x+1; if (r < rows && c < cols) { subCommands.push_back({r, c}); }
			r = y  ; c = x+2; if (r < rows && c < cols) { subCommands.push_back({r, c}); }

			std::shuffle(subCommands.begin(), subCommands.end(), rng);
			commands.insert(commands.end(), subCommands.begin(), subCommands.end());

			r = y+1; c = x+1; if (r < rows && c < cols) { commands.push_back({r, c}); }
		}
	}
	return commands;
}

Commands box3(int rows, int cols) {
	Commands commands;
    for (int y = 0; y < rows; y += 3) {
        for (int x = 0; x < cols; x += 3) {
			int r, c;
			r = y  ; c = x  ; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y+1; c = x  ; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y+2; c = x  ; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y+2; c = x+1; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y+2; c = x+2; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y+1; c = x+2; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y  ; c = x+2; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y  ; c = x+1; if (r < rows && c < cols) { commands.push_back({r, c}); }
			r = y+1; c = x+1; if (r < rows && c < cols) { commands.push_back({r, c}); }
		}
	}
	return commands;
}

Commands randomMap(int rows, int cols, std::mt19937& rng) {
    std::vector<Command> commands;
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            commands.push_back({y, x});
        }
    }
    std::shuffle(begin(commands), end(commands), rng);

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
	std::mt19937 rng(seed);

	Commands commands;
	if (type == "rowmajor") {
		commands = rowMajor(rows, cols);
	} else if (type == "checker") {
		commands = checkerBoard(rows, cols);
	} else if (type == "random") {
		commands = randomMap(rows, cols, rng);
	} else if (type == "snail") {
		commands = snail(rows, cols);
	} else if (type == "box3") {
		commands = box3(rows, cols);
	} else if (type == "randombox3") {
		commands = randomBox3(rows, cols, rng);
	} else {
		std::cerr << "Unknown type" << std::endl;
		return 1;
	}

	std::cout << applyCommands(rows, cols, commands) << std::endl;
}
