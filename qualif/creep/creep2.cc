#include "creep2.hh"
#include "MC.hpp"

void executeCommand(game& g, const Command& command) {
    if (command.command <= 0) {
        g.tick();
    } else {
        int cmd = command.command;
        int id = command.id;
        int x = command.x;
        int y = command.y;
        if (cmd == 1) {
            g.queen_spawn_creep_tumor(g.get_queen(id),pos(x,y));
        } else if (cmd == 2) {
            g.creep_tumor_spawn_creep_tumor(g.get_creep_tumor(id),pos(x,y));
        } else {
            assert(0 && "invalid cmd code");
        }
    }
}

std::vector<Command> LoadCommands(const std::string& file) {
    std::vector<Command> commands;
    std::ifstream in(file);

    int count;
    in >> count;
    int last_command = 0;
    for (int i = 0; i < count; ++i) {
        Command cmd;
        in >> cmd.t >> cmd.command >> cmd.id >> cmd.x >> cmd.y;
        for (int j = last_command; j < cmd.t; ++j) {
            commands.push_back(Command{});
            commands.back().t = j;
            last_command = cmd.t;
        }
        commands.push_back(cmd);
    }
    return commands;
}

int main(int argc, char **argv) {
    assert(argc == 2 || argc == 3);

    auto g = std::make_unique<game>(argv[1]);
    std::vector<Command> commands;
    std::vector<Command> redoCommands;

    if (argc == 3) {
        commands = LoadCommands(argv[2]);
    }

    for (auto& cmd : commands) {
        assert(cmd.t == g->t_q2);
        executeCommand(*g, cmd);
    }

    gui::Game gui(GuiModelFromGame(*g));

    auto commandCallback = [&](const Command& copy) {
        redoCommands.clear();
        auto command = copy;
        command.t = g->t_q2;
        commands.push_back(command);
        executeCommand(*g, command);
        gui.setModel(GuiModelFromGame(*g));
    };

    gui.setCommandCallback(commandCallback);
    gui.setUndoCallback([&]() {
        while (!commands.empty() && commands.back().command <= 0) {
            redoCommands.push_back(commands.back());
            commands.pop_back();
        }
        if (!commands.empty()) {
            redoCommands.push_back(commands.back());
            commands.pop_back();
        }
        g = std::make_unique<game>(argv[1]);
        for (auto& command : commands) {
            executeCommand(*g, command);
        }
        gui.setModel(GuiModelFromGame(*g));
    });
    gui.setRedoCallback([&]() {
        while (!redoCommands.empty() && redoCommands.back().command <= 0) {
            commands.push_back(redoCommands.back());
            executeCommand(*g, redoCommands.back());
            redoCommands.pop_back();
        }
        if (!redoCommands.empty()) {
            commands.push_back(redoCommands.back());
            executeCommand(*g, redoCommands.back());
            redoCommands.pop_back();
        }
        gui.setModel(GuiModelFromGame(*g));
    });
    gui.setAutoCallback([&]() {
        MonteCarlo mc(g.get());
        auto cmd = mc.getAutoMove();
        commandCallback(cmd);
    });
    gui.run();

    std::vector<Command> final_commands;
    for (auto& command : commands) {
        if (command.command > 0) {
            final_commands.push_back(command);
        }
    }
    std::cout << final_commands.size() << std::endl;
    for (auto& command : final_commands) {
        printf("%d %d %d %d %d\n",
            command.t, command.command, command.id, command.x, command.y);
    }

    return 0;
}

Model GuiModelFromGame(game& game) {
    TileMatrix tiles(boost::extents[game.map_dx][game.map_dy]);

    for (int y = 0; y < game.map_dy; ++y) {
        for (int x = 0; x < game.map_dx; ++x) {
            auto* building = game.map_building[y][x];
            if (game.map_wall[y][x]) {
                tiles[x][y] = Wall{};
            } else if (building) {
                if (building->name() == std::string("creep_tumor")) {
                    auto* ct = static_cast<creep_tumor*>(building);
                    CreepTumor creepTumor;
                    creepTumor.id = ct->id;
                    if (!ct->spawn_creep_tumor_active) {
                        creepTumor.state = CreepTumor::State::InActive;
                    } else if (ct->dt_spawn_creep_tumor_cooldown_q8 > 0) {
                        creepTumor.state = CreepTumor::State::Cooldown;
                        creepTumor.cooldown =
                            ct->dt_spawn_creep_tumor_cooldown_q8 / dt_tick_q8;
                    } else {
                        creepTumor.state = CreepTumor::State::Active;
                    }
                    tiles[x][y] = creepTumor;
                } else {
                    tiles[x][y] = Hatchery{};
                }
            } else if (game.map_creep[y][x]) {
                tiles[x][y] = Creep{};
            } else if (0 < game.map_creep_gen[y][x]) {
                if (game.creep_spread_candidate(pos(x, y))) {
                    tiles[x][y] = CreepCandidate{};
                } else {
                    tiles[x][y] = CreepRadius{};
                }
            }
        }
    }

    std::vector<Queen> queens;

    for (auto* unit : game.units) {
        auto* q = static_cast<queen*>(unit);
        Queen queen;
        queen.id = q->id;
        queen.energy = q->energy_q8;
        queens.push_back(queen);
    }

    Model model {
        game.t_q2,
        game.t_limit_q2,
        tiles,
        queens
    };
    model.game = &game;
    return model;
}

std::vector<pos> GetQueenSpawnablePositions(game& game) {
    std::vector<pos> result;
    for(uint y = 0; y < game.map_dy; ++y) {
        for(uint x = 0; x < game.map_dx; ++x) {
            auto p = pos(x, y);
            if (game.valid_pos(p) &&
                !game.map_wall[p.y][p.x] &&
                !game.map_building[p.y][p.x] &&
                game.map_creep[p.y][p.x])
            {
                result.push_back(p);
            }
        }
    }
    return result;
}

Command GetCommand(game& game) {
    for (auto* building : game.buildings) {
        if (building->name() == std::string("creep_tumor")) {
            auto ct = static_cast<creep_tumor*>(building);
            if (!ct->spawn_creep_tumor_active) {
                // inactive
            } else if (ct->dt_spawn_creep_tumor_cooldown_q8 > 0) {
                // cooldown
            } else {
                // available
            }
        }
    }

    for (auto* unit : game.units) {
        if (unit->name() == std::string("queen")) {
            auto q = static_cast<queen*>(unit);
            if (q->energy_q8 < queen::spawn_creep_tumor_energy_cost_q8) {
                // charging
            } else {
                auto spawnable_pos = GetQueenSpawnablePositions(game);
                auto p = spawnable_pos[rand() % spawnable_pos.size()];

                return Command::QueenSpawn(q->id, p.x, p.y);
            }
        }
    }
    return Command{};
}
