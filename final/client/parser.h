#pragma once
#include "stdafx.h"


static const int HATCHERY_BUILD_QUEEN_COST = 24000;
static const int HATCHERY_MAX_ENERGY = 80000;
static const int HATCHERY_MAX_HP = 1500;
static const int QUEEN_BUILD_CREEP_TUMOR_COST = 100;
static const int QUEEN_MAX_ENERGY = 200;
static const int QUEEN_MAX_HP = 175;
static const int CREEP_TUMOR_SPAWN_ENERGY = 60;
static const int CREEP_TUMOR_MAX_HP = 200;
static const int QUEEN_DAMAGE = 40;
static const int HP_DECAY_ON_ENEMY_CREEP = 2;
static const int HP_REGEN_ON_FRIENDLY_CREEP = 2;
static const int ENERGY_REGEN = 1;
static const int MAX_QUEENS = 8;
static const int HATCHERY_SIZE = 3;
static const int MAX_TICK = 1200;

enum class UnitType {
	kHatchery,
	kEnemyHatchery,
	kCreepTumor,
	kEnemyCreepTumor,
	kQueen,
	kEnemyQueen
};

inline bool IsBuilding(UnitType t) {
	return t == UnitType::kHatchery ||
		t == UnitType::kEnemyHatchery ||
		t == UnitType::kCreepTumor ||
		t == UnitType::kEnemyCreepTumor;
}

struct POS
{
	enum eDirection
	{
		SHIFT_UP = 0,
		SHIFT_RIGHT = 1,
		SHIFT_DOWN = 2,
		SHIFT_LEFT = 3
	};
	int x, y;
	POS() { x=y=0; }
	POS(int _x, int _y) { x=_x; y=_y;}
	bool operator== (POS const &rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}
	bool operator!= (POS const &rhs) const
	{
		return x != rhs.x || y != rhs.y;
	}
	bool operator< (POS const &rhs) const
	{
		if (y != rhs.y)
			return y<rhs.y;
		return x<rhs.x;
	}
	bool IsNear(POS const &rhs) const
	{
		return abs(x-rhs.x)+abs(y-rhs.y)<=1;
	}
	bool IsValid() const
	{
		return x!=0;
	}
	POS ShiftXY(int dx, int dy) { return POS(x+dx, y+dy); }
	POS ShiftDir(int dir)
	{
		switch (dir)
		{
		case SHIFT_UP: return POS(x,y-1);
		case SHIFT_RIGHT: return POS(x+1,y);
		case SHIFT_DOWN: return POS(x,y+1);
		default: return POS(x-1,y);
		}
	}
};

std::ostream& operator<<(std::ostream& os, const POS& p);

struct MAP_OBJECT {
	int id, hp, energy, side;
	POS pos;

        bool IsEnemy() const {
            return side != 0;
        }
};

class PARSER
{
public:
	PARSER();
	int tick;
	int versus[2]; // your ID and enemy ID. versus[1]==0 if not real opponent (test)
	enum eGroundType
	{
		EMPTY,
		WALL,
		CREEP,
		CREEP_CANDIDATE_FRIENDLY,
		CREEP_CANDIDATE_ENEMY,
		CREEP_CANDIDATE_BOTH,
		ENEMY_CREEP
	};
	int w, h;
	std::vector<eGroundType> Arena;
	std::vector<MAP_OBJECT> Units;
	MAP_OBJECT OwnHatchery;
	MAP_OBJECT EnemyHatchery;
	std::vector<MAP_OBJECT> CreepTumors;

	eGroundType GetAt(const POS &p) const;
	void ParseUnits(const std::vector<std::string> &ServerResponse, int &index, int count, std::vector<MAP_OBJECT> &container);
	enum eMatchResult {
		ONGOING,
		VICTORY,
		DRAW,
		DEFEAT
	};
	eMatchResult match_result;
	void Parse(const std::vector<std::string> &ServerResponse);

	std::pair<UnitType, MAP_OBJECT*> GetUnitAt(const POS& pos);
	MAP_OBJECT* FindUnit(int id);
};
