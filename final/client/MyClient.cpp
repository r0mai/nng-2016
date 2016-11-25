#include "stdafx.h"
#include "Client.h"
#include "parser.h"
#include "fleepath.h"
#include <cmath>

// sample
//
enum class BuildingType {
	kHatchery,
	kEnemyHatchery,
	kCreepTumor,
	kEnemyCreepTumor
};

class MYCLIENT : public CLIENT {
public:
	MYCLIENT();
protected:
	virtual std::string GetPassword() { return std::string("47JdZX"); }
	virtual std::string GetPreferredOpponents() { return opponent; }
	virtual bool NeedDebugLog() { return true; }
	virtual void Process();

	void PreprocessUnitTargets();
	int ClosestTumorDistance(const POS& pos, int side = 0);

	int GetTumorFitness(const POS& p);
	POS GetBestCreep();
	std::vector<POS> GetCellsInRadius(const POS& pos, int radius = 10);
	int GetEmptyCountAround(const POS& pos);
	int GetEnemyCreepCountAround(const POS& pos);
	bool CanPlaceTumor(const POS& pos);
	int Distance(const POS& p1, const POS& p2);

	std::pair<BuildingType, MAP_OBJECT*> GetBuildingAt(const POS& pos);
};

MYCLIENT::MYCLIENT() {}

void MYCLIENT::PreprocessUnitTargets() {
	std::vector<int> to_clear;
	for (auto& p : mUnitTarget) {
		if (p.second.c == CMD_SPAWN) {
			if (!CanPlaceTumor(p.second.pos)) {
				to_clear.push_back(p.first);
			}
		}
	}
	for (auto id : to_clear) {
		mUnitTarget.erase(id);
	}
}

void MYCLIENT::Process() {
	PreprocessUnitTargets();

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

int MYCLIENT::GetTumorFitness(const POS& p) {
	if (!CanPlaceTumor(p)) {
		return -1;
	}

	auto empty_count = GetEmptyCountAround(p);
	auto enemy_creep_count = GetEnemyCreepCountAround(p);
	return enemy_creep_count + 2 * empty_count + ClosestTumorDistance(p);
}

POS MYCLIENT::GetBestCreep() {
	POS best_pos = POS(-1, -1);
	int best_fit = -1;
	for (int y = 0; y < mParser.h; ++y) {
		for (int x = 0; x < mParser.w; ++x) {
			POS p(x, y);
			auto fitness = GetTumorFitness(p);
			if (fitness > best_fit) {
				best_fit = fitness;
				best_pos = p;
			}
		}
	}
	std::cout << "Best count = " << best_fit << " @ " << best_pos << std::endl;
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

int MYCLIENT::GetEnemyCreepCountAround(const POS& pos) {
	int count = 0;
	for (auto& p : GetCellsInRadius(pos)) {
		if (mParser.GetAt(p) == PARSER::ENEMY_CREEP) {
			++count;
		}
	}
	return count;
}

std::pair<BuildingType, MAP_OBJECT*> MYCLIENT::GetBuildingAt(const POS& pos) {
	if (pos == mParser.EnemyHatchery.pos) {
		return {BuildingType::kEnemyHatchery, &mParser.EnemyHatchery};
	}
	if (pos == mParser.OwnHatchery.pos) {
		return {BuildingType::kHatchery, &mParser.OwnHatchery};
	}
	for (auto& ct : mParser.CreepTumors) {
		if (pos == ct.pos) {
			return {ct.side == 0 ? BuildingType::kCreepTumor : BuildingType::kEnemyCreepTumor, &ct};
		}
	}
	return {{}, nullptr};
}


bool MYCLIENT::CanPlaceTumor(const POS& pos) {
	return mParser.GetAt(pos) == PARSER::CREEP && GetBuildingAt(pos).second == nullptr;
}

int MYCLIENT::GetEmptyCountAround(const POS& pos) {
	int count = 0;
	for (auto& p : GetCellsInRadius(pos)) {
		if (mParser.GetAt(p) == PARSER::EMPTY || mParser.GetAt(p) == PARSER::ENEMY_CREEP) {
			++count;
		}
	}
	return count;
}

int MYCLIENT::Distance(const POS& p1, const POS& p2) {
	int dx = p1.x - p2.x;
	int dy = p1.y - p2.y;
	return int(ceil(sqrt(dx * dx + dy * dy)));
}

int MYCLIENT::ClosestTumorDistance(const POS& pos, int side) {
	int dst = INT_MAX;
	for (const auto& obj : mParser.CreepTumors) {
		if (obj.side != side) {
			continue;
		}
		dst = std::min(dst, Distance(obj.pos, pos));
	}
	return dst;
}

CLIENT *CreateClient() {
	return new MYCLIENT();
}
