#include "Game.hpp"
#include <iostream>
#include <boost/algorithm/string/join.hpp>

namespace gui {

Game::Game(const Model& model) :
    window(sf::VideoMode(1024, 1024), "Title"),
    model(model)
{}

void Game::run() {
    while (window.isOpen()) {
        handleEvents();
        draw();
    }
}

void Game::setModel(const Model& model) {
    this->model = model;
}

void Game::setCommandCallback(const CommandCallback& callback) {
    commandCallback = callback;
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
            case sf::Event::KeyPressed:
                handleKeyPressedEvent(event.key);
                break;
            case sf::Event::MouseMoved:
                handleMouseMovedEvent(event.mouseMove);
                break;
            default:
                break;
        }
    }
}

void Game::handleMouseButtonPressedEvent(const sf::Event::MouseButtonEvent& ev) {
    auto p = windowToTile(ev.x, ev.y);
    if (isValidPosition(p)) {
        clickOn(p);
    }
}

sf::Vector2i Game::windowToTile(int wx, int wy) const {
    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    float width = window.getSize().x;
    float height = window.getSize().y;

    if (wx < 0 || wx >= int(width) ||
        wy < 0 || wy >= int(height))
    {
        return {-1, -1};
    }

    int x = wx / (width / float(columns));
    int y = wy / (height / float(rows));

    return sf::Vector2i{x, y};
}

void Game::handleKeyPressedEvent(const sf::Event::KeyEvent& ev) {
    switch (ev.code) {
        case sf::Keyboard::N:
            if (!hasValidMove()) {
                sendCommand(Command{});
            } else {
                std::cerr << "You have a valid move (P to force)" << std::endl;
            }
            break;
        case sf::Keyboard::P:
            sendCommand(Command{});
            break;
        case sf::Keyboard::Q:
            inputMode = InputMode::QueenSpawn;
            activeTumorPos = {-1, -1};
            break;
        default:
            break;
    }
}

void Game::handleMouseMovedEvent(const sf::Event::MouseMoveEvent& ev) {
    highlights.clear();
    auto p = windowToTile(ev.x, ev.y);
    if (isValidPosition(p)) {
        highlights = cellsAround(p, 10);
    }
}

void Game::clickOn(const sf::Vector2i& p) {
    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    auto* creepTumor = boost::get<CreepTumor>(&model.tiles[p.x][p.y]);
    auto* creep = boost::get<Creep>(&model.tiles[p.x][p.y]);
    if (creepTumor) {
        if (creepTumor->state == CreepTumor::State::Active) {
            inputMode = InputMode::TumorSpawn;
            activeTumorPos = sf::Vector2i{p.x, p.y};
        }
    }
    if (creep) {
        if (inputMode == InputMode::TumorSpawn) {
            if (!isValidPosition(activeTumorPos)) {
                std::cerr << "Invalid active tumor coordinate" << std::endl;
                return;
            }
            auto spawnTumor = boost::get<CreepTumor>(
                &model.tiles[activeTumorPos.x][activeTumorPos.y]);

            if (!spawnTumor) {
                std::cerr << "Target is not a tumor" << std::endl;
                return;
            }

            if (spawnTumor->state != CreepTumor::State::Active) {
                std::cerr << "CreepTumor not active" << std::endl;
                return;
            }

            auto candidateCells = cellsAround(activeTumorPos, 10);

            auto it = std::find(begin(candidateCells), end(candidateCells), p);
            if (it == end(candidateCells)) {
                std::cerr << "New position too far away" << std::endl;
                return;
            }

            sendCommand(Command::TumorSpawn(spawnTumor->id, p.x, p.y));
            activeTumorPos = {-1, -1};
        } else if (inputMode == InputMode::QueenSpawn) {
            for (auto& queen : model.queens) {
                if (queen.energy >= 6400) {
                    sendCommand(Command::QueenSpawn(queen.id, p.x, p.y));
                    break;
                }
            }
        }
    }
}

bool Game::isValidPosition(const sf::Vector2i& p) const {
    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    return p.x >= 0 && p.y >= 0 && p.x < columns && p.y < rows;
}

std::vector<sf::Vector2i> Game::cellsAround(const sf::Vector2i& p, int radius) const {
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

bool Game::hasValidMove() const {
    for (auto& queen : model.queens) {
        if (queen.energy >= 6400) {
            return true;
        }
    }

    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    for (auto y = 0; y < rows; ++y) {
        for (auto x = 0; x < columns; ++x) {
            auto* creepTumor = boost::get<CreepTumor>(&model.tiles[x][y]);
            if (creepTumor && creepTumor->state == CreepTumor::State::Active) {
                return true;
            }
        }
    }
    return false;
}

void Game::sendCommand(const Command& cmd) {
    if (commandCallback) {
        commandCallback(cmd);
        inputMode = InputMode::QueenSpawn;
    }
}

std::string Game::GetStatusString() const {
    std::stringstream ss;

    ss << "Mode: ";
    if (inputMode == InputMode::QueenSpawn) {
        ss << "q";
    } else {
        ss << "t";
    }

    ss << " ";

    ss << "Queens: (";
    if (!model.queens.empty()) {
        ss << model.queens.front().energy;
        for (int i = 1; i < model.queens.size(); ++i) {
            ss << "," << model.queens[i].energy;
        }
    }
    ss << ")";

    ss << " ";

    ss << "(" << model.tick << "/" << model.max_tick << ")";

    return ss.str();
}

struct Game::TileDrawer : boost::static_visitor<> {
    TileDrawer(
        Game& game,
        const sf::Vector2i& position) :
        game(game),
        position(position)
    {}

    void operator()(const Hatchery&) {
        game.drawTile(position, sf::Color::Magenta);
        game.drawSmallTile(position, sf::Color{202, 31, 123});
    }

    void operator()(const Wall&) {
        game.drawTile(position, sf::Color{169, 169, 169});
    }

    void operator()(const Empty&) {
        game.drawTile(position, sf::Color::Black);
    }

    void operator()(const Creep&) {
        game.drawTile(position, sf::Color::Magenta);
    }

    void operator()(const CreepTumor& ct) {
        game.drawTile(position, sf::Color::Magenta);
        switch (ct.state) {
            case CreepTumor::State::Active:
                game.drawSmallTile(position, sf::Color::Green);
                break;
            case CreepTumor::State::Cooldown:
                game.drawSmallTile(position, sf::Color::Yellow);
                break;
            case CreepTumor::State::InActive:
                game.drawSmallTile(position, sf::Color{202, 31, 123});
                break;
        }
    }

    void operator()(const CreepCandidate&) {
        game.drawTile(position, sf::Color::Black);
        game.drawSmallTile(position, sf::Color::Magenta);
    }

    void operator()(const CreepRadius&) {
        game.drawTile(position, sf::Color::Black);
        game.drawSmallTile(position, sf::Color::Magenta);
    }

private:
    Game& game;
    sf::Vector2i position;
};

void Game::drawTile(const sf::Vector2i& p, const sf::Color& color) {
    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    float width = window.getSize().x;
    float height = window.getSize().y;

    sf::Vector2f tileTopLeft{p.x * width/columns, p.y * height/rows};
    sf::Vector2f tileSize{width/columns, height/rows};
    sf::RectangleShape rect(tileSize);

    rect.setPosition(tileTopLeft);
    rect.setFillColor(color);
    window.draw(rect);
}

void Game::drawSmallTile(const sf::Vector2i& p, const sf::Color& color) {
    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    float width = window.getSize().x;
    float height = window.getSize().y;

    sf::Vector2f tileTopLeft{p.x * width/columns, p.y * height/rows};
    sf::Vector2f tileSize{width/columns, height/rows};

    sf::RectangleShape rect(tileSize * 0.5f);
    rect.setPosition(tileTopLeft + 0.25f * tileSize);
    rect.setFillColor(color);
    window.draw(rect);
}

void Game::draw() {
    window.clear();

    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    float width = window.getSize().x;
    float height = window.getSize().y;

    for (auto y = 0; y < rows; ++y) {
        for (auto x = 0; x < columns; ++x) {
            TileDrawer tileDrawer{*this, {x, y}};
            boost::apply_visitor(tileDrawer, model.tiles[x][y]);
        }
    }
    if (isValidPosition(activeTumorPos)) {
        drawTile(activeTumorPos, sf::Color{0, 0, 0, 128});
    }
    for (auto& p : highlights) {
        drawTile(p, sf::Color{255, 255, 66, 128});
    }

    window.setTitle(GetStatusString());
    window.display();
}

} // namespace gui
