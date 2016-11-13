#include "MC.hpp"
#include "Util.hpp"

#include <tuple>
#include <thread>
#include <future>

using MCResult = std::tuple<sf::Vector2i, int>;
using MCResults = std::vector<MCResult>;
using MCResultsFuture = std::future<MCResults>;

MonteCarlo::MonteCarlo(game* g) : g(g->clone()), rngs(4) {}

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

        auto job = [&](int ti) {
            MCResults result;
            auto cs = candidates.size();
            int from = ti * cs / THREAD_COUNT;
            int to = (ti + 1) * cs / THREAD_COUNT;
            for (int i = from; i < cs && i < to; ++i) {
                auto& candidate = candidates[i];
                auto base = g->clone();
                executeCommand(*base, Command::TumorSpawn(tumorId, candidate.x, candidate.y));
                int score = doMCRun(base.get(), ti);

                result.push_back({candidate, score});

                std::cerr << "(" << ti << ") " << "T Candidate " << candidate
                    << " " << i+1 << "/" << candidates.size()
                    << " score = " << score << std::endl;
            }
            return result;
        };

        std::vector<MCResultsFuture> resultFutures;

        for (int i = 0; i < THREAD_COUNT; ++i) {
            resultFutures.push_back(std::async(std::launch::async, job, i));
        }
        MCResults results;
        for (int i = 0; i < THREAD_COUNT; ++i) {
            MCResults result = resultFutures[i].get();
            results.insert(results.end(), result.begin(), result.end());
        }

        MCResult bestResult = {sf::Vector2i{-1, -1}, std::numeric_limits<int>::max()};
        for (auto& result : results) {
            if (std::get<1>(result) < std::get<1>(bestResult)) {
                bestResult = result;
            }
        }

        return Command::TumorSpawn(
            tumorId, std::get<0>(bestResult).x, std::get<0>(bestResult).y);
    } else if (model.hasQueenMove()) {
        int queenId = model.getFreeQueenId();
        auto candidates = model.getEdgeCells();
        sf::Vector2i target;
        if (candidates.empty()) {
            std::cerr << "no candidate for queen move" << std::endl;
            auto fb_pos = model.justACreepCell();
            return Command::QueenSpawn(queenId, fb_pos.x, fb_pos.y);
        }

        auto job = [&](int ti) {
            MCResults result;
            auto cs = candidates.size();
            int from = ti * cs / THREAD_COUNT;
            int to = (ti + 1) * cs / THREAD_COUNT;
            for (int i = from; i < cs && i < to; ++i) {
                auto& candidate = candidates[i];
                auto base = g->clone();
                executeCommand(*base, Command::QueenSpawn(queenId, candidate.x, candidate.y));
                int score = doMCRun(base.get(), ti);

                result.push_back({candidate, score});

                std::cerr << "(" << ti << ") " << "Q Candidate " << candidate
                    << " " << i+1 << "/" << candidates.size()
                    << " score = " << score << std::endl;
            }
            return result;
        };

        std::vector<MCResultsFuture> resultFutures;

        for (int i = 0; i < THREAD_COUNT; ++i) {
            resultFutures.push_back(std::async(std::launch::async, job, i));
        }
        MCResults results;
        for (int i = 0; i < THREAD_COUNT; ++i) {
            MCResults result = resultFutures[i].get();
            results.insert(results.end(), result.begin(), result.end());
        }

        MCResult bestResult = {sf::Vector2i{-1, -1}, std::numeric_limits<int>::max()};
        for (auto& result : results) {
            if (std::get<1>(result) < std::get<1>(bestResult)) {
                bestResult = result;
            }
        }

        return Command::QueenSpawn(
            queenId, std::get<0>(bestResult).x, std::get<0>(bestResult).y);
    } else {
        return Command{};
    }
}

int MonteCarlo::doMCRun(game* base, int thread_index) {
    int score = 0;
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
                    target = candidates[dis(rngs[thread_index])];
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
                    target = candidates[dis(rngs[thread_index])];
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
