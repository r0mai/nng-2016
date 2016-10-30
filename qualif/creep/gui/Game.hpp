#pragma once

#include "../Model.hpp"
#include <SFML/Graphics.hpp>

namespace gui {

class Game {
public:
    using CommandCallback = std::function<void(const Command& cmd)>;

    Game(const Model& model);

    void run();
    void setModel(const Model& model);
    void setCommandCallback(const CommandCallback& callback);

private:
    void handleEvents();
    void handleMouseButtonPressedEvent(const sf::Event::MouseButtonEvent& ev);
    void handleKeyPressedEvent(const sf::Event::KeyEvent& ev);
    void clickOn(int x, int y);

    void drawTile(const sf::Vector2i& p, const sf::Color& color);
    void drawSmallTile(const sf::Vector2i& p, const sf::Color& color);
    void draw();

    bool isValidPosition(const sf::Vector2i& p) const;
    std::vector<sf::Vector2i> cellsAround(const sf::Vector2i& p, int radius) const;

    bool hasValidMove() const;

    void sendCommand(const Command& cmd);

    std::string GetStatusString() const;

    enum class InputMode {
        QueenSpawn,
        TumorSpawn
    };
    InputMode inputMode = InputMode::QueenSpawn;
    sf::Vector2i activeTumorPos{-1, -1};

    Model model;

    CommandCallback commandCallback;

    sf::RenderWindow window;

    struct TileDrawer;
};

} // namespace gui
