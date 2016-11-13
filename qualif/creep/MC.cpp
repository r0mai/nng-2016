#include "MC.hpp"
#include "Util.hpp"

MonteCarlo::MonteCarlo(game* g) : g(g->clone()) {}

Command MonteCarlo::getAutoMove() {
    auto model = GuiModelFromGame(*g);

    auto tumorPos = model.hasTumorMove();
    if (model.isValidPosition(tumorPos)) {
        int tumorId = boost::get<CreepTumor>(&model.tiles[tumorPos.x][tumorPos.y])->id;
        auto candidates = model.getEdgeCellsAround(tumorPos, 10);
        if (candidates.empty()) {
            std::cerr << "no candidate for tumor move" << std::endl;
            auto fb_pos = model.justACreepCellAround(tumorPos, 10);
            return Command::TumorSpawn(tumorId, fb_pos.x, fb_pos.y);
        }
        sf::Vector2i best_candidate;
        int lowest_score = std::numeric_limits<int>::max();
        for (int i = 0; i < candidates.size(); ++i) {
            auto& candidate = candidates[i];
            std::cerr << "T Candidate " << candidate << " " << i+1 << "/"
                << candidates.size() << ", best = " << best_candidate
                << ", score = " << lowest_score << std::endl;
            auto base = g->clone();
            executeCommand(*base, Command::TumorSpawn(tumorId, candidate.x, candidate.y));
            int score = doMCRun(base.get());
            if (score < lowest_score) {
                lowest_score = score;
                best_candidate = candidate;
            }
        }
        return Command::TumorSpawn(tumorId, best_candidate.x, best_candidate.y);
    } else if (model.hasQueenMove()) {
        int queenId = model.getFreeQueenId();
        auto candidates = model.getEdgeCells();
        sf::Vector2i target;
        if (candidates.empty()) {
            std::cerr << "no candidate for queen move" << std::endl;
            auto fb_pos = model.justACreepCell();
            return Command::QueenSpawn(queenId, fb_pos.x, fb_pos.y);
        }
        sf::Vector2i best_candidate;
        int lowest_score = std::numeric_limits<int>::max();
        for (int i = 0; i < candidates.size(); ++i) {
            auto& candidate = candidates[i];
            std::cerr << "Q Candidate " << candidate << " " << i+1 << "/"
                << candidates.size() << ", best = " << best_candidate
                << ", score = " << lowest_score << std::endl;
            auto base = g->clone();
            executeCommand(*base, Command::QueenSpawn(queenId, candidate.x, candidate.y));
            int score = doMCRun(base.get());
            if (score < lowest_score) {
                lowest_score = score;
                best_candidate = candidate;
            }
        }
        return Command::QueenSpawn(queenId, best_candidate.x, best_candidate.y);
    } else {
        return Command{};
    }
}

int MonteCarlo::doMCRun(game* base) {
    int score = 0.0;
    for (int i = 0; i < 100; ++i) {
        auto mc_game = base->clone();
        while (true) {
            if (mc_game->t_q2 >= 1000 || !mc_game->has_empty()) {
                score += mc_game->t_q2;
                break;
            }
            if (!mc_game->hasValidMove()) {
                executeCommand(*mc_game, Command{});
                continue;
            }

            auto model = GuiModelFromGame(*mc_game);
            auto tumorPos = model.hasTumorMove();
            if (model.isValidPosition(tumorPos)) {
                auto candidates = model.getEdgeCellsAround(tumorPos, 10);
                sf::Vector2i target;
                if (candidates.empty()) {
                    target = model.justACreepCellAround(tumorPos, 10);
                } else {
                    std::uniform_int_distribution<> dis(0, candidates.size() - 1);
                    target = candidates[dis(rng)];
                }
                executeCommand(*mc_game,
                    Command::TumorSpawn(
                        boost::get<CreepTumor>(&model.tiles[tumorPos.x][tumorPos.y])->id,
                        target.x, target.y
                    )
                );
            } else if (model.hasQueenMove()) {
                auto candidates = model.getEdgeCells();
                sf::Vector2i target;
                if (candidates.empty()) {
                    target = model.justACreepCell();
                } else {
                    std::uniform_int_distribution<> dis(0, candidates.size() - 1);
                    target = candidates[dis(rng)];
                }
                executeCommand(*mc_game,
                    Command::QueenSpawn(
                        model.getFreeQueenId(),
                        target.x, target.y
                    )
                );
            } else {
                assert(false);
            }
        }
    }
    return score;
}
