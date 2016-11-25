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
    for (int y = 0; y < mParser.h; ++y) {
        for (int x = 0; x < mParser.w; ++x) {
            auto tile = mParser.GetAt(POS(x, y));
            if (tile == PARSER::CREEP) {
                return POS(x, y);
            }
        }
    }
    return POS(-1, -1);
}

CLIENT *CreateClient() {
    return new MYCLIENT();
}
