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
