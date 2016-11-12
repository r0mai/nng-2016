#pragma once

#include "../Model.hpp"
#include <SFML/Graphics.hpp>

namespace gui {

class Game {
public:
    using CommandCallback = std::function<void(const Command& cmd)>;
    using UndoCallback = std::function<void()>;
    using RedoCallback = std::function<void()>;

    Game(const Model& model);

    void run();
    void setModel(const Model& model);
    void setCommandCallback(const CommandCallback& callback);
    void setUndoCallback(const UndoCallback& callback);
    void setRedoCallback(const RedoCallback& callback);

private:
    void handleEvents();
    void handleMouseButtonPressedEvent(const sf::Event::MouseButtonEvent& ev);
    void handleKeyPressedEvent(const sf::Event::KeyEvent& ev);
    void handleMouseMovedEvent(const sf::Event::MouseMoveEvent& ev);
    void clickOn(const sf::Vector2i& p);
    void selectNextTumor(bool forward = true);
    void toggleBeacon();

    void drawTile(const sf::Vector2i& p, const sf::Color& color);
    void drawSmallTile(const sf::Vector2i& p, const sf::Color& color);
    void draw();

    sf::Vector2i windowToTile(int wx, int wy) const;

    void sendCommand(const Command& cmd);

    std::string GetStatusString() const;

    enum class InputMode {
        QueenSpawn,
        TumorSpawn
    };
    InputMode inputMode = InputMode::QueenSpawn;
    sf::Vector2i activeTumorPos{-1, -1};
    std::vector<sf::Vector2i> highlights;
    std::vector<sf::Vector2i> beacons;
    sf::Vector2i cursor{-1, -1};

    Model model;

    CommandCallback commandCallback;
    UndoCallback undoCallback;
    RedoCallback redoCallback;

    sf::RenderWindow window;

    struct TileDrawer;
};

} // namespace gui
