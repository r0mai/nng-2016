#include "stdafx.h"
#include "Client.h"
#include "parser.h"
#include "fleepath.h"
#include <cmath>

#include <boost/range/adaptor/filtered.hpp>

// sample
//

class MYCLIENT : public CLIENT {
public:
	MYCLIENT();
protected:
	virtual std::string GetPassword() { return std::string("47JdZX"); }
	virtual std::string GetPreferredOpponents() { return opponent; }
	virtual bool NeedDebugLog() { return true; }
	virtual void Process();

	int last_hatchery_energy_ = 0;

	void PrintStatistics();

	void AttackAttackingQueens();
	void SpawnOrAttackWithQueens();
	void SpawnWithTumors();
	void AttackHatchery();

	void PreprocessUnitTargets();
	int ClosestTumorDistance(const POS& pos, bool enemy = false);

	int GetTumorFitness(const POS& p);
	int GetEnemyTumorFitness(const POS& pos, int energy);

	POS GetBestCreep();
	std::vector<POS> GetCellsInRadius(const POS& pos, int radius = 10);
	int GetEmptyCountAround(const POS& pos);
	int GetEnemyCreepCountAround(const POS& pos);
	bool CanPlaceTumor(const POS& pos);
	bool HasTentativeTumorAt(const POS& pos);
	int Distance(const POS& p1, const POS& p2);
	int RouteDistance(const POS& p1, const POS& p2);
	std::vector<MAP_OBJECT> GetOurQueens();
	std::vector<MAP_OBJECT> GetEnemyQueens();
	std::vector<MAP_OBJECT> GetEnemyTumors();

	bool CanWeDie() const;
	bool DoWeHaveMoreCreep() const;
};

MYCLIENT::MYCLIENT() {}

void MYCLIENT::PrintStatistics() {
	std::cout << "H gained " <<
		mParser.OwnHatchery.energy - last_hatchery_energy_ << "e" <<
		" (" << mParser.OwnHatchery.energy << "e)" << std::endl;
	last_hatchery_energy_ = mParser.OwnHatchery.energy;
}

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

void MYCLIENT::AttackAttackingQueens() {
	for (auto& enemy_queen : GetEnemyQueens()) {
		if (mParser.GetAt(enemy_queen.pos) == PARSER::CREEP) {
			for (auto& queen : GetOurQueens()) {
				mUnitTarget[queen.id].c = CMD_ATTACK;
				mUnitTarget[queen.id].target_id = enemy_queen.id;
			}
			return;
		}
	}
}

void MYCLIENT::SpawnOrAttackWithQueens() {
	FLEEPATH FleePath;
	FleePath.CreateCreepDist(&mParser);
	for (auto& queen : GetOurQueens()) {
		if (mUnitTarget.count(queen.id)) {
			continue;
		}
		if (queen.energy >= QUEEN_BUILD_CREEP_TUMOR_COST) {
			POS creep = GetBestCreep();
			if (creep.IsValid()) {
				mUnitTarget[queen.id].c = CMD_SPAWN;
				mUnitTarget[queen.id].pos = creep;
			}
		} else {
			int best_fit = INT_MIN;
			int best_id = mParser.EnemyHatchery.id;
			for (auto& tumor : GetEnemyTumors()) {
				auto dst = RouteDistance(queen.pos, tumor.pos);
				auto fitness = GetEnemyTumorFitness(tumor.pos, tumor.energy);
				fitness -= dst;
				if (fitness > best_fit) {
					best_fit = fitness;
					best_id = tumor.id;
				}
			}

			mUnitTarget[queen.id].c = CMD_ATTACK;
			mUnitTarget[queen.id].target_id = best_id;
		}
	}
}

void MYCLIENT::SpawnWithTumors() {
	const auto& ourTumors = mParser.CreepTumors |
			boost::adaptors::filtered(
					[](const MAP_OBJECT& tumor) { return tumor.side == 0; });
	const auto& activeTumors = ourTumors | boost::adaptors::filtered(
			[](const MAP_OBJECT& tumor) {
					return tumor.energy >= CREEP_TUMOR_SPAWN_ENERGY; });

	for (const auto& tumor : activeTumors) {
		auto cells = GetCellsInRadius(tumor.pos);
		auto best = std::max_element(cells.rbegin(), cells.rend(),
				[this](const POS& l, const POS& r) {
					// will select last with the same fitness
					return GetTumorFitness(l) < GetTumorFitness(r);
				});
		if (best != cells.rend()) {
			command_buffer <<  "creep_tumor_spawn" << " " <<
				tumor.id << " " <<
				best->x << " " <<
				best->y << std::endl;

		}
	}
}

void MYCLIENT::AttackHatchery() {
	for (auto& queen : GetOurQueens()) {
		mUnitTarget[queen.id].c = CMD_ATTACK;
		mUnitTarget[queen.id].target_id = mParser.EnemyHatchery.id;
	}
}

void MYCLIENT::Process() {
	PrintStatistics();

	PreprocessUnitTargets();
	AttackAttackingQueens();
	SpawnOrAttackWithQueens();
	SpawnWithTumors();
	if (GetOurQueens().size() >= 7) {
		AttackHatchery();
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

int MYCLIENT::GetEnemyTumorFitness(const POS& pos, int energy) {
	int our_dst = ClosestTumorDistance(pos, false);
	int enemy_dst = ClosestTumorDistance(pos, true);
	int fitness = enemy_dst - our_dst;

	if (energy > 0) {
		fitness += energy / 4;
	}

	return fitness;
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
	std::sort(cells.begin(), cells.end(), [](const POS& p, const POS& q){
		auto sp = p.x + p.y;
		auto sq = q.x + q.y;
		return sp < sq || (sp == sq && p.x < q.x);
	});

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

bool MYCLIENT::HasTentativeTumorAt(const POS& pos) {
	for (auto& p : mUnitTarget) {
		if (p.second.c == CMD_SPAWN && p.second.pos == pos) {
			return true;
		}
	}
	return false;
}

bool MYCLIENT::CanPlaceTumor(const POS& pos) {
	auto unit = mParser.GetUnitAt(pos);
	return mParser.GetAt(pos) == PARSER::CREEP &&
		(unit.second == nullptr || !IsBuilding(unit.first)) &&
		!HasTentativeTumorAt(pos);
}

int MYCLIENT::GetEmptyCountAround(const POS& pos) {
	int count = 0;
	for (auto& p : GetCellsInRadius(pos)) {
		if (mParser.GetAt(p) == PARSER::EMPTY ||
			mParser.GetAt(p) == PARSER::CREEP_CANDIDATE_ENEMY)
		{
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

int MYCLIENT::RouteDistance(const POS& p1, const POS& p2) {
	return mDistCache.GetDist(p1, p2);
}

int MYCLIENT::ClosestTumorDistance(const POS& pos, bool enemy) {
	int dst = Distance(pos, (enemy ? EnemyHatchery : OwnHatchery).pos);

	for (const auto& obj : mParser.CreepTumors) {
		if (obj.IsEnemy() != enemy || obj.pos == pos) {
			continue;
		}
		dst = std::min(dst, Distance(obj.pos, pos));
	}
	return dst;
}

std::vector<MAP_OBJECT> MYCLIENT::GetOurQueens() {
	std::vector<MAP_OBJECT> queens;
	queens.reserve(8);
	for (auto& enemy_queen : mParser.Units) {
		if (!enemy_queen.IsEnemy()) {
			queens.push_back(enemy_queen);
		}
	}
	return queens;
}

std::vector<MAP_OBJECT> MYCLIENT::GetEnemyQueens() {
	std::vector<MAP_OBJECT> queens;
	queens.reserve(8);
	for (auto& queen : mParser.Units) {
		if (queen.IsEnemy()) {
			queens.push_back(queen);
		}
	}
	return queens;
}

std::vector<MAP_OBJECT> MYCLIENT::GetEnemyTumors() {
	std::vector<MAP_OBJECT> tumors;
	for (auto& e : mParser.CreepTumors) {
		if (e.IsEnemy()) {
			tumors.push_back(e);
		}
	}
	return tumors;
}


bool MYCLIENT::CanWeDie() const {
	auto remainingTicks = MAX_TICK - mParser.tick;
	const auto numberOfEnemyUnits = 8;
	// They might manufacture more than they currently have, so 8 is an upper
	// bound.
	// TODO: Calculate the number of queens they could create and deploy
	auto damagePossible = remainingTicks * numberOfEnemyUnits * QUEEN_DAMAGE;
	return damagePossible >= mParser.OwnHatchery.hp;
}

bool MYCLIENT::DoWeHaveMoreCreep() const {
	const auto& arena = mParser.Arena;
	std::size_t ourCreep = std::count(arena.begin(), arena.end(),
			PARSER::CREEP);
	std::size_t theirCreep = std::count(arena.begin(), arena.end(),
			PARSER::ENEMY_CREEP);
	return ourCreep > theirCreep;
}

CLIENT *CreateClient() {
	return new MYCLIENT();
}
