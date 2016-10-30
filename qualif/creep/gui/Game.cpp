#include "Game.hpp"
#include <iostream>

namespace gui {

Game::Game(const TileMatrix& model) :
    window(sf::VideoMode(768, 768), "Title"),
    model(model)
{}

void Game::run() {
    while (window.isOpen()) {
        handleEvents();
        draw();
    }
}

void Game::setModel(const TileMatrix& model) {
    this->model = model;
}

void Game::handleEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::MouseButtonPressed:
                handleMouseButtonPressedEvent(event.mouseButton);
                break;
            default:
                break;
        }
    }
}

void Game::handleMouseButtonPressedEvent(const sf::Event::MouseButtonEvent& ev) {

}

struct TileDrawer : boost::static_visitor<> {
    TileDrawer(
        sf::RenderWindow& window,
        const sf::Vector2f& tileTopLeft,
        const sf::Vector2f& tileSize) :
        window(window),
        tileTopLeft(tileTopLeft),
        tileSize(tileSize)
    {}

    void drawTile(const sf::Color& color) {
        sf::RectangleShape rect(tileSize);
        rect.setPosition(tileTopLeft);
        rect.setFillColor(color);
        window.draw(rect);
    }

    void operator()(const Hatchery&) {
        drawTile(sf::Color{202, 31, 123});
    }

    void operator()(const Wall&) {
        drawTile(sf::Color{169, 169, 169});
    }

    void operator()(const Plain&) {
        drawTile(sf::Color::Black);
    }

    void operator()(const Creep&) {
        drawTile(sf::Color::Magenta);
    }

private:
    sf::RenderWindow& window;
    sf::Vector2f tileTopLeft;
    sf::Vector2f tileSize;
};

void Game::draw() {
    window.clear();

    auto columns = model.shape()[0];
    auto rows = model.shape()[1];

    float width = window.getSize().x;
    float height = window.getSize().y;

    for (auto y = 0; y < rows; ++y) {
        for (auto x = 0; x < columns; ++x) {
            TileDrawer tileDrawer{
                window,
                sf::Vector2f{x * width/columns, y * height/rows},
                sf::Vector2f{width/columns, height/rows}
            };
            boost::apply_visitor(tileDrawer, model[x][y]);
        }
    }
    window.display();
}

} // namespace gui
