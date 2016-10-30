#pragma once

#include "../Model.hpp"
#include <SFML/Graphics.hpp>

class Game {
public:
    Game(const TileMatrix& model);

    void run();
    void setModel(const TileMatrix& model);
private:
    void handleEvents();
    void handleMouseButtonPressedEvent(const sf::Event::MouseButtonEvent& ev);

    void draw();

    TileMatrix model;

    sf::RenderWindow window;
};
