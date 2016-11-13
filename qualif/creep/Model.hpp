#pragma once

#include <SFML/Graphics.hpp>

#include <boost/variant.hpp>
#include <boost/multi_array.hpp>

struct game;

struct Hatchery {};

struct Wall {};

struct Empty {};

struct Creep {};

struct CreepTumor {
    enum class State {
        InActive,
        Cooldown,
        Active
    };

    int id = -1;
    int cooldown = 0;
    State state;
};

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

struct Queen {
    int id = -1;
    int energy = -1;
};

enum class TumorSpawnResult {
    SUCCESS = 0,
    SOURCE_NOT_TUMOR = 1,
    DEST_NOT_CREEP = 2,
    INVALID_POSITION = 3,
    TUMOR_NOT_ACTIVE = 4,
    DEST_TOO_FAR_AWAY = 5
};

struct Model {
    Model() = default;
    Model(int tick, int max_tick, const TileMatrix& tiles, const std::vector<Queen>& queens) :
        tick(tick), max_tick(max_tick), tiles(tiles), queens(queens) {}
    int tick = -1;
    int max_tick = -1;
    TileMatrix tiles;
    std::vector<Queen> queens;
    game* game = nullptr;

    bool isValidPosition(const sf::Vector2i& p) const;

    bool hasValidMove() const;
    bool hasQueenMove() const;
    sf::Vector2i hasTumorMove() const;

    int getCoveredCount() const;
    int getEmptyCount() const;

    int getFreeQueenId() const;

    std::vector<sf::Vector2i> cellsAround(const sf::Vector2i& p, int radius) const;

    bool isCreepEdgeCell(const sf::Vector2i& p) const;
    std::vector<sf::Vector2i> getEdgeCells() const;
    std::vector<sf::Vector2i> getEdgeCellsAround(const sf::Vector2i& p, int radius) const;

    sf::Vector2i justACreepCell() const;
    sf::Vector2i justACreepCellAround(const sf::Vector2i& p, int radius) const;

    TumorSpawnResult canTumowSpawn(const sf::Vector2i& from, const sf::Vector2i& to) const;
};

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

    int t = -1;
    int command = -1;
    int id;
    int x;
    int y;
};
