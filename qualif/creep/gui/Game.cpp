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
            default:
                break;
        }
    }
}

void Game::handleMouseButtonPressedEvent(const sf::Event::MouseButtonEvent& ev) {
    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    float width = window.getSize().x;
    float height = window.getSize().y;

    if (ev.x < 0 || ev.x >= int(width) ||
        ev.y < 0 || ev.y >= int(height))
    {
        return;
    }

    int x = ev.x / (width / float(columns));
    int y = ev.y / (height / float(rows));

    clickOn(x, y);
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
            break;
        default:
            break;
    }
}

void Game::clickOn(int x, int y) {
    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    auto* creepTumor = boost::get<CreepTumor>(&model.tiles[x][y]);
    auto* creep = boost::get<Creep>(&model.tiles[x][y]);
    if (creepTumor) {
        if (creepTumor->state == CreepTumor::State::Active) {
            inputMode = InputMode::TumorSpawn;
            activeTumorPos = sf::Vector2i{x, y};
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

            auto it = std::find(begin(candidateCells), end(candidateCells), activeTumorPos);
            if (it == end(candidateCells)) {
                std::cerr << "New position too far away" << std::endl;
                return;
            }

            sendCommand(Command::TumorSpawn(spawnTumor->id, x, y));
        } else if (inputMode == InputMode::QueenSpawn) {
            for (auto& queen : model.queens) {
                if (queen.energy >= 6400) {
                    sendCommand(Command::QueenSpawn(queen.id, x, y));
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
        ss << "queen";
    } else {
        ss << "tumor";
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

    return ss.str();
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

    void drawInnerTile(const sf::Color& color) {
        sf::RectangleShape rect(tileSize * 0.5f);
        rect.setPosition(tileTopLeft + 0.25f * tileSize);
        rect.setFillColor(color);
        window.draw(rect);
    }

    void operator()(const Hatchery&) {
        drawTile(sf::Color::Magenta);
        drawInnerTile(sf::Color{202, 31, 123});
    }

    void operator()(const Wall&) {
        drawTile(sf::Color{169, 169, 169});
    }

    void operator()(const Empty&) {
        drawTile(sf::Color::Black);
    }

    void operator()(const Creep&) {
        drawTile(sf::Color::Magenta);
    }

    void operator()(const CreepTumor& ct) {
        drawTile(sf::Color::Magenta);
        switch (ct.state) {
            case CreepTumor::State::Active:
                drawInnerTile(sf::Color::Green);
                break;
            case CreepTumor::State::Cooldown:
                drawInnerTile(sf::Color::Yellow);
                break;
            case CreepTumor::State::InActive:
                drawInnerTile(sf::Color{202, 31, 123});
                break;
        }
    }

    void operator()(const CreepCandidate&) {
        drawTile(sf::Color::Black);
        drawInnerTile(sf::Color::Magenta);
    }

    void operator()(const CreepRadius&) {
        drawTile(sf::Color::Black);
        drawInnerTile(sf::Color::Magenta);
    }

private:
    sf::RenderWindow& window;
    sf::Vector2f tileTopLeft;
    sf::Vector2f tileSize;
};

void Game::draw() {
    window.clear();

    auto columns = model.tiles.shape()[0];
    auto rows = model.tiles.shape()[1];

    float width = window.getSize().x;
    float height = window.getSize().y;

    for (auto y = 0; y < rows; ++y) {
        for (auto x = 0; x < columns; ++x) {
            TileDrawer tileDrawer{
                window,
                sf::Vector2f{x * width/columns, y * height/rows},
                sf::Vector2f{width/columns, height/rows}
            };
            boost::apply_visitor(tileDrawer, model.tiles[x][y]);
        }
    }
    window.setTitle(GetStatusString());
    window.display();
}

} // namespace gui
