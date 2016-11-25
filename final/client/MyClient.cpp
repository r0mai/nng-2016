#include "stdafx.h"
#include "Client.h"
#include "parser.h"
#include "fleepath.h"
#include <cmath>
#include <numeric>
#include <unordered_set>
#include <chrono>

#include <boost/range/algorithm_ext/erase.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/transformed.hpp>

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
	void ReactToHeatMap();
	int ClosestTumorDistance(const POS& pos, bool enemy = false);

	int GetTumorFitness(const POS& p);
	int GetEnemyTumorFitness(const POS& pos, int energy);

	POS GetBestCreepWithQueen(const POS& pos);
	std::vector<POS> GetCellsInRadius(const POS& pos, int radius = 10);
	int GetEmptyCountAround(const POS& pos);
	int GetEnemyCreepCountAround(const POS& pos);
	bool CanPlaceTumor(const POS& pos);
	bool HasTentativeTumorAt(const POS& pos);
	int Distance(const POS& p1, const POS& p2);
	int RouteDistance(const POS& p1, const POS& p2);
	int GetAttackTarget(const POS& pos, int force);
	int GetEnemyThreat(const POS& pos);
	int GetForce(const POS& pos);
	int GetHeat(const POS& pos);

	std::vector<MAP_OBJECT> GetOurQueens();
	std::vector<MAP_OBJECT> GetOurNonFleeingQueens();
	std::vector<MAP_OBJECT> GetEnemyQueens();
	std::vector<MAP_OBJECT> GetIntrudingQueens();
	std::vector<MAP_OBJECT> GetEnemyTumors();
	std::vector<MAP_OBJECT> GetOurTumors();

	bool CanWeDie();
	bool DoWeHaveMoreCreep();
	bool AreWeStrongerBy(float ratio = 1.0);
	static int queenToHealth(const MAP_OBJECT& queen) {
		return queen.hp;
	}
	bool OnOurCreep(const MAP_OBJECT& queen) const {
		return mParser.GetAt(queen.pos) == PARSER::CREEP;
	}

	const MAP_OBJECT* GetClosestEnemyNear(const POS& pos);

	std::unordered_set<int> fleeing_queens;
};

MYCLIENT::MYCLIENT() {}

void MYCLIENT::PrintStatistics() {
	for (int y = 0; y < mParser.w; ++y) {
		for (int x = 0; x < mParser.h; ++x) {
			if (mParser.GetAt(POS{x, y}) == PARSER::WALL) {
				std::cout << "█";
				continue;
			}
			if (mParser.GetAt(POS{x, y}) == PARSER::EMPTY) {
				std::cout << " ";
				continue;
			}
			auto units = mParser.GetUnitsAt(POS{x, y});
			std::cout << " ";
		}
		std::cout << std::endl;
	}
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
	auto enemyQueens = GetIntrudingQueens();
	if (enemyQueens.empty()) { return; }
	for (auto& queen : GetOurNonFleeingQueens()) {
		mUnitTarget[queen.id].c = CMD_ATTACK_MOVE;
		auto queenToAttack = GetClosestEnemyNear(queen.pos);
		if (queenToAttack) {
			mUnitTarget[queen.id].target_id = queenToAttack->id;
		}
	}
}

void MYCLIENT::SpawnOrAttackWithQueens() {
	FLEEPATH flee_path;
	flee_path.CreateCreepDist(&mParser);
	for (auto& queen : GetOurNonFleeingQueens()) {
		if (mUnitTarget.count(queen.id)) {
			continue;
		}
		int empty_around = 0;

		const auto& ourTumors = GetOurTumors();
		if (queen.energy >= QUEEN_BUILD_CREEP_TUMOR_COST || ourTumors.empty()) {
			POS creep = GetBestCreepWithQueen(queen.pos);
			if (creep.IsValid()) {
				empty_around = GetEmptyCountAround(creep);
				mUnitTarget[queen.id].c = CMD_SPAWN;
				mUnitTarget[queen.id].pos = creep;
			}
		}

		if (!empty_around) {
			auto force = GetForce(queen.pos);
			auto target = GetAttackTarget(queen.pos, force);
			if (target != -1) {
				mUnitTarget[queen.id].c = CMD_ATTACK_MOVE;
				mUnitTarget[queen.id].target_id = target;
			} else {
				mUnitTarget[queen.id].c = CMD_MOVE;
				mUnitTarget[queen.id].pos = flee_path.GetNextOffCreep(queen.pos);
			}
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

int MYCLIENT::GetEnemyThreat(const POS& pos) {
	int sum = 0;
	int max_dst = 10;
	for (const auto& queen : GetEnemyQueens()) {
		auto dst = RouteDistance(queen.pos, pos);
		auto t = mParser.GetAt(queen.pos);
		float mul = 1.0;
		if (t == PARSER::ENEMY_CREEP) { mul = 1.25; }
		else if (t == PARSER::CREEP) { mul = 0.8; }
		if (dst < max_dst) {
			auto rs = queen.hp / 40 + 1;
			sum += (max_dst - dst) * rs * mul;
		}
	}
	return sum;
}

int MYCLIENT::GetForce(const POS& pos) {
	int sum = 0;
	int max_dst = 10;
	for (const auto& queen : GetOurQueens()) {
		auto dst = RouteDistance(queen.pos, pos);
		auto t = mParser.GetAt(queen.pos);
		float mul = 1.0;
		if (t == PARSER::CREEP) { mul = 1.25; }
		else if (t == PARSER::ENEMY_CREEP) { mul = 0.8; }
		if (dst < max_dst) {
			auto rs = queen.hp / 40 + 1;
			sum += (max_dst - dst) * rs * mul;
		}
	}
	return sum;
}

int MYCLIENT::GetHeat(const POS& pos) {
	return GetForce(pos) - GetEnemyThreat(pos);
}

int MYCLIENT::GetAttackTarget(const POS& pos, int force) {
	int best_id = -1;
	int best_fit = INT_MIN;

	for (const auto& tumor : GetEnemyTumors()) {
		auto dst = RouteDistance(pos, tumor.pos);
		auto threat = GetEnemyThreat(tumor.pos);
		auto fitness = GetEnemyTumorFitness(tumor.pos, tumor.energy);

		fitness -= dst;
		fitness -= threat;

		if (fitness > best_fit) {
			best_fit = fitness;
			best_id = tumor.id;
		}
	}

	for (const auto& queen : GetEnemyQueens()) {
		auto dst = RouteDistance(queen.pos, pos);
		auto threat = GetEnemyThreat(queen.pos);
		int fitness = -50; // fitness modifier

		fitness -= dst;
		fitness -= threat;
		if (fitness > best_fit) {
			best_fit = fitness;
			best_id = queen.id;
		}
	}

	if (GetEnemyQueens().size() <= GetOurQueens().size()) {
		auto hatchery = mParser.EnemyHatchery;
		auto dst = RouteDistance(pos, hatchery.pos);
		auto threat = GetEnemyThreat(hatchery.pos);
		auto fitness = - dst - threat;

		if (fitness > best_fit) {
			best_fit = fitness;
			best_id = hatchery.id;
		}
	}

	if (best_fit + force > 0) {
		return best_id;
	}

	return -1;
}

void MYCLIENT::AttackHatchery() {
	for (auto& queen : GetOurNonFleeingQueens()) {
		mUnitTarget[queen.id].c = CMD_ATTACK_MOVE;
		mUnitTarget[queen.id].target_id = mParser.EnemyHatchery.id;
	}
}

void MYCLIENT::ReactToHeatMap() {
	for (auto& queen : GetOurQueens()) {
		if (GetHeat(queen.pos) < -200) {
			auto cells = GetCellsInRadius(queen.pos, 2);

			boost::remove_erase_if(cells, [this](const POS& p) {
				return mParser.GetAt(p) == PARSER::WALL;
			});
			auto best = std::max_element(cells.begin(), cells.end(),
				[this](const POS& lhs, const POS& rhs) {
					return GetHeat(lhs) < GetHeat(rhs);
				}
			);
			mUnitTarget[queen.id].c = CMD_MOVE;
			mUnitTarget[queen.id].pos = *best;
			fleeing_queens.insert(queen.id);
		}
	}
}

void MYCLIENT::Process() {
	fleeing_queens.clear();
	auto start_t = std::chrono::system_clock::now();
	PrintStatistics();

	PreprocessUnitTargets();
	ReactToHeatMap();
	AttackAttackingQueens();
	SpawnOrAttackWithQueens();
	SpawnWithTumors();
	if (GetOurNonFleeingQueens().size() >= 6) {
		AttackHatchery();
	}

	auto end_t = std::chrono::system_clock::now();
	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
		end_t - start_t).count();

	std::cout << diff << "ms" << std::endl;
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

	//if (energy > 0) {
	//	fitness += energy / 4;
	//}

	return fitness;
}

POS MYCLIENT::GetBestCreepWithQueen(const POS& pos) {
	POS best_pos = POS(-1, -1);
	int best_fit = INT_MIN;
	for (int y = 0; y < mParser.h; ++y) {
		for (int x = 0; x < mParser.w; ++x) {
			POS p(x, y);
			auto fitness = 10 * GetTumorFitness(p) - RouteDistance(pos, p);
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
	if (mParser.GetAt(pos) != PARSER::CREEP) {
		return false;
	}
	if (HasTentativeTumorAt(pos)) {
		return false;
	}
	for (auto& unit : mParser.GetUnitsAt(pos)) {
		if (IsBuilding(unit.first)) {
			return false;
		}
	}
	return true;
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
	auto hatchery = (enemy ? mParser.EnemyHatchery : mParser.OwnHatchery);
	int dst = Distance(pos, hatchery.pos);

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

std::vector<MAP_OBJECT> MYCLIENT::GetOurNonFleeingQueens() {
	std::vector<MAP_OBJECT> queens;
	queens.reserve(8);
	for (auto& enemy_queen : mParser.Units) {
		if (!enemy_queen.IsEnemy() && !fleeing_queens.count(enemy_queen.id)) {
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

std::vector<MAP_OBJECT> MYCLIENT::GetIntrudingQueens() {
	auto enemyQueens = GetEnemyQueens();

	std::vector<MAP_OBJECT> intrudingQueens;
	for (const auto& queen: enemyQueens) {
		if (OnOurCreep(queen) || RouteDistance(mParser.OwnHatchery.pos, queen.pos) < 25) {
			intrudingQueens.push_back(queen);
		}
	}
	return intrudingQueens;
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

std::vector<MAP_OBJECT> MYCLIENT::GetOurTumors() {
	std::vector<MAP_OBJECT> tumors;
	for (auto& e : mParser.CreepTumors) {
		if (!e.IsEnemy()) {
			tumors.push_back(e);
		}
	}
	return tumors;
}


bool MYCLIENT::CanWeDie() {
	auto remainingTicks = MAX_TICK - mParser.tick;
	const auto numberOfEnemyUnits = 8;
	// They might manufacture more than they currently have, so 8 is an upper
	// bound.
	// TODO: Calculate the number of queens they could create and deploy
	auto damagePossible = remainingTicks * numberOfEnemyUnits * QUEEN_DAMAGE;
	return damagePossible >= mParser.OwnHatchery.hp;
}

bool MYCLIENT::DoWeHaveMoreCreep() {
	const auto& arena = mParser.Arena;
	std::size_t ourCreep = std::count(arena.begin(), arena.end(),
			PARSER::CREEP);
	std::size_t theirCreep = std::count(arena.begin(), arena.end(),
			PARSER::ENEMY_CREEP);
	return ourCreep > theirCreep;
}

bool MYCLIENT::AreWeStrongerBy(float ratio) {
	const auto& ourHealths = GetOurQueens() | boost::adaptors::transformed(
			queenToHealth);
	const auto& theirHealths = GetEnemyQueens() | boost::adaptors::transformed(
			queenToHealth);
	auto ourHealth = std::accumulate(ourHealths.begin(), ourHealths.end(), 0);
	auto theirHealth = std::accumulate(theirHealths.begin(), theirHealths.end(),
			0);
	return ratio * ourHealth > theirHealth;
}

const MAP_OBJECT* MYCLIENT::GetClosestEnemyNear(const POS& pos) {
	auto enemyQueens = GetIntrudingQueens();
	auto it = std::min_element(enemyQueens.begin(), enemyQueens.end(),
			[this, pos](const MAP_OBJECT& l, const MAP_OBJECT& r) mutable {
				return RouteDistance(l.pos, pos) < RouteDistance(r.pos, pos);
			});
	if (it != enemyQueens.end()) {
		return &*it;
	}
	return nullptr;
}

CLIENT *CreateClient() {
	return new MYCLIENT();
}
