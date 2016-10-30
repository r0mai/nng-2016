#include <iostream>

#include "Game.hpp"

int main() {
    TileMatrix model(boost::extents[64][64]);
    model[10][2] = Creep{};

    gui::Game game{model};
    game.run();
}
