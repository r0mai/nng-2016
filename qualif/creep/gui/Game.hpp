#pragma once

#include "../Model.hpp"
#include <SFML/Graphics.hpp>

namespace gui {

class Game {
public:
    using CommandCallback = std::function<void(const Command& cmd)>;

    Game(const TileMatrix& model);

    void run();
    void setModel(const TileMatrix& model);
    void setCommandCallback(const CommandCallback& callback);

private:
    void handleEvents();
    void handleMouseButtonPressedEvent(const sf::Event::MouseButtonEvent& ev);

    void draw();

    TileMatrix model;

    CommandCallback commandCallback;

    sf::RenderWindow window;
};

} // namespace gui
