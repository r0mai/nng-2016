#pragma once

#include "creep2.hh"
#include "Model.hpp"

class MonteCarlo {
public:
    MonteCarlo(game* g);

private:
    std::unique_ptr<game> g;
};
