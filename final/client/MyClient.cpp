#include "stdafx.h"
#include "Client.h"
#include "parser.h"
#include "fleepath.h"

// sample

class MYCLIENT : public CLIENT {
public:
	MYCLIENT();
protected:
	virtual std::string GetPassword() { return std::string("47JdZX"); }
	virtual std::string GetPreferredOpponents() { return std::string("test"); }
	virtual bool NeedDebugLog() { return true; }
	virtual void Process();

        POS GetBestCreep();
        std::vector<POS> GetCellsInRadius(const POS& pos, int radius = 10);
        int GetEmptyCountAround(const POS& pos);
};

MYCLIENT::MYCLIENT() {}

void MYCLIENT::Process() {
    FLEEPATH FleePath;
    FleePath.CreateCreepDist(&mParser);
    for (auto& queen : mParser.Units) {
        if (queen.side != 0) {
            continue;
        }

        if (!mUnitTarget.count(queen.id)) {
            POS creep = GetBestCreep();
    	    if (creep.IsValid()) {
    	    	mUnitTarget[queen.id].c = CMD_SPAWN;
    	    	mUnitTarget[queen.id].pos = creep;
    	    }
        }

    }
}

POS MYCLIENT::GetBestCreep() {
    POS best_pos = POS(-1, -1);
    int best_count = -1;
    for (int y = 0; y < mParser.h; ++y) {
        for (int x = 0; x < mParser.w; ++x) {
            POS p(x, y);
            auto tile = mParser.GetAt(p);
            if (tile != PARSER::CREEP) {
                continue;
            }

            auto empty_count = GetEmptyCountAround(p);
            if (empty_count > best_count) {
                best_count = empty_count;
                best_pos = p;
            }
        }
    }
    return best_pos;
}

std::vector<POS> MYCLIENT::GetCellsInRadius(const POS& pos, int radius) {
    std::vector<POS> cells;
    for (int dy=-radius+1; dy<radius; ++dy) {
	for (int dx=-radius+1; dx<radius; ++dx) {
	    POS p(pos.x+dx,pos.y+dy);
            int dx_q1=2*dx+(0<dx?1:-1);
	    int dy_q1=2*dy+(0<dy?1:-1);
	    int d2_q2=dx_q1*dx_q1+dy_q1*dy_q1;
	    if (d2_q2<=radius*radius*4) {
	        cells.push_back(p);
            }
        }
    }
    return cells;
}

int MYCLIENT::GetEmptyCountAround(const POS& pos) {
    int count = 0;
    for (auto& p : GetCellsInRadius(pos)) {
        if (mParser.GetAt(p) == PARSER::EMPTY) {
            ++count;
        }
    }
    return count;
}

CLIENT *CreateClient() {
    return new MYCLIENT();
}
