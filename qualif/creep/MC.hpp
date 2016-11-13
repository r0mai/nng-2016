#pragma once

#include "creep2.hh"
#include "Model.hpp"

#include <random>

class MonteCarlo {
public:
    MonteCarlo(game* g);

    Command getAutoMove();
private:
    int doMCRun(game* base);

    std::unique_ptr<game> g;

    std::minstd_rand rng;
};
