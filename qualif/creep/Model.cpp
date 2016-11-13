#include "Model.hpp"

bool Model::hasValidMove() const {
    return hasQueenMove() || isValidPosition(hasTumorMove());
}

bool Model::hasQueenMove() const {
    for (auto& queen : queens) {
        if (queen.energy >= 6400) {
            return true;
        }
    }
    return false;
}

bool Model::isValidPosition(const sf::Vector2i& p) const {
    auto columns = tiles.shape()[0];
    auto rows = tiles.shape()[1];

    return p.x >= 0 && p.y >= 0 && p.x < columns && p.y < rows;
}


sf::Vector2i Model::hasTumorMove() const {
    auto columns = tiles.shape()[0];
    auto rows = tiles.shape()[1];

    for (auto y = 0; y < rows; ++y) {
        for (auto x = 0; x < columns; ++x) {
            auto* creepTumor = boost::get<CreepTumor>(&tiles[x][y]);
            if (creepTumor && creepTumor->state == CreepTumor::State::Active) {
                return sf::Vector2i{x, y};
            }
        }
    }
    return sf::Vector2i{-1, -1};
}

int Model::getCoveredCount() const {
    auto columns = tiles.shape()[0];
    auto rows = tiles.shape()[1];

    int covered = 0;
    for (auto y = 0; y < rows; ++y) {
        for (auto x = 0; x < columns; ++x) {
            auto& t = tiles[x][y];
            if (boost::get<CreepTumor>(&t) ||
                boost::get<Creep>(&t) ||
                boost::get<Hatchery>(&t))
            {
                ++covered;
            }
        }
    }
    return covered;
}

int Model::getEmptyCount() const {
    auto columns = tiles.shape()[0];
    auto rows = tiles.shape()[1];

    int empty = 0;
    for (auto y = 0; y < rows; ++y) {
        for (auto x = 0; x < columns; ++x) {
            auto& t = tiles[x][y];
            if (boost::get<Empty>(&t)) {
                ++empty;
            }
        }
    }
    return empty;
}

std::vector<sf::Vector2i> Model::cellsAround(const sf::Vector2i& p, int radius) const {
    std::vector<sf::Vector2i> cells;
    for (int dy = -radius+1; dy < radius; ++dy) {
        for(int dx = -radius+1; dx < radius; ++dx) {
            sf::Vector2i cell(p.x + dx, p.y + dy);
            if (!isValidPosition(cell)) {
                continue;
            }

            int dx_q1 = 2*dx+(0<dx?1:-1);
            int dy_q1 = 2*dy+(0<dy?1:-1);
            int d2_q2 = dx_q1*dx_q1 + dy_q1*dy_q1;
            if (d2_q2 <= radius*radius*4) {
                cells.push_back(cell);
            }
        }
    }
    return cells;
}

bool Model::isCreepEdgeCell(const sf::Vector2i& p) const {
    if (!boost::get<Creep>(&tiles[p.x][p.y])) {
        return false;
    }
    if (boost::get<Empty>(&tiles[p.x+1][p.y]) ||
        boost::get<Empty>(&tiles[p.x-1][p.y]) ||
        boost::get<Empty>(&tiles[p.x][p.y+1]) ||
        boost::get<Empty>(&tiles[p.x][p.y-1]))
    {
        return true;
    }
    return false;
}

std::vector<sf::Vector2i> Model::getEdgeCells() const {
    auto columns = tiles.shape()[0];
    auto rows = tiles.shape()[1];

    std::vector<sf::Vector2i> cells;
    cells.reserve(50);

    for (auto y = 1; y < rows-1; ++y) {
        for (auto x = 1; x < columns-1; ++x) {
            if (isCreepEdgeCell({x, y})) {
                cells.push_back({x, y});
            }
        }
    }

    return cells;
}

std::vector<sf::Vector2i> Model::getEdgeCellsAround(const sf::Vector2i& p, int radius) const {
    std::vector<sf::Vector2i> cells;
    for (int dy = -radius+1; dy < radius; ++dy) {
        for(int dx = -radius+1; dx < radius; ++dx) {
            sf::Vector2i cell(p.x + dx, p.y + dy);
            if (!isValidPosition(cell) || !isCreepEdgeCell(cell)) {
                continue;
            }

            int dx_q1 = 2*dx+(0<dx?1:-1);
            int dy_q1 = 2*dy+(0<dy?1:-1);
            int d2_q2 = dx_q1*dx_q1 + dy_q1*dy_q1;
            if (d2_q2 <= radius*radius*4) {
                cells.push_back(cell);
            }
        }
    }
    return cells;
}

int Model::getFreeQueenId() const {
    for (auto& queen : queens) {
        if (queen.energy >= 6400) {
            return queen.id;
        }
    }
    return -1;
}

sf::Vector2i Model::justACreepCell() const {
    auto columns = tiles.shape()[0];
    auto rows = tiles.shape()[1];

    for (auto y = 2; y < rows; ++y) {
        for (auto x = 2; x < columns; ++x) {
            auto& t = tiles[x][y];
            if (boost::get<Creep>(&t)) {
                return {x, y};
            }
        }
    }
    return {-1, -1};
}

sf::Vector2i Model::justACreepCellAround(const sf::Vector2i& p, int radius) const {
    for (int dy = -radius+1; dy < radius; ++dy) {
        for(int dx = -radius+1; dx < radius; ++dx) {
            sf::Vector2i cell(p.x + dx, p.y + dy);
            if (!isValidPosition(cell)) {
                continue;
            }

            int dx_q1 = 2*dx+(0<dx?1:-1);
            int dy_q1 = 2*dy+(0<dy?1:-1);
            int d2_q2 = dx_q1*dx_q1 + dy_q1*dy_q1;
            if (d2_q2 <= radius*radius*4 && boost::get<Creep>(&tiles[cell.x][cell.y])) {
                return cell;
            }
        }
    }
    return {-1, -1};
}

TumorSpawnResult Model::canTumowSpawn(const sf::Vector2i& from, const sf::Vector2i& to) const {
    if (!isValidPosition(from) || !isValidPosition(to)) {
        return TumorSpawnResult::INVALID_POSITION;
    }
    auto* sourceTumor = boost::get<CreepTumor>(&tiles[from.x][from.y]);
    auto* destCreep = boost::get<Creep>(&tiles[to.x][to.y]);
    if (!sourceTumor) {
        return TumorSpawnResult::SOURCE_NOT_TUMOR;
    }
    if (!destCreep) {
        return TumorSpawnResult::DEST_NOT_CREEP;
    }

    if (sourceTumor->state != CreepTumor::State::Active) {
        return TumorSpawnResult::TUMOR_NOT_ACTIVE;
    }

    auto candidateCells = cellsAround(from, 10);

    auto it = std::find(begin(candidateCells), end(candidateCells), to);
    if (it == end(candidateCells)) {
        return TumorSpawnResult::DEST_TOO_FAR_AWAY;
    }

    return TumorSpawnResult::SUCCESS;
}
