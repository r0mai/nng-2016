#pragma once

#include <boost/variant.hpp>
#include <boost/multi_array.hpp>

struct Hatchery {};
struct Wall {};
struct Empty {};
struct Creep {};
struct CreepTumor {};
struct CreepCandidate {};
struct CreepRadius {};

using Tile = boost::variant<
    Empty,
    Wall,
    Hatchery,
    CreepTumor,
    Creep,
    CreepCandidate,
    CreepRadius
>;

using TileMatrix = boost::multi_array<Tile, 2>;

struct Command {
    Command() = default;

    static Command QueenSpawn(int id, int x, int y) {
        Command cmd;
        cmd.command = 1;
        cmd.id = id;
        cmd.x = x;
        cmd.y = y;
        return cmd;
    }

    static Command TumorSpawn(int id, int x, int y) {
        Command cmd;
        cmd.command = 2;
        cmd.id = id;
        cmd.x = x;
        cmd.y = y;
        return cmd;
    }

    int command = -1;
    int id;
    int x;
    int y;
};
