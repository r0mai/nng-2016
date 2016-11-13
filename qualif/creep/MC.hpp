#pragma once

#include "creep2.hh"
#include "Model.hpp"

#include <random>

class MonteCarlo {
public:
    MonteCarlo(game* g);

    Command getAutoMove();
private:
    static const int THREAD_COUNT = 4;

    int doMCRun(game* base, int thread_index);

    std::unique_ptr<game> g;

    static std::vector<std::minstd_rand> rngs;
};
