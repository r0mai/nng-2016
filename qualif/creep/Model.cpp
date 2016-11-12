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
