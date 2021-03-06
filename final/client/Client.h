#pragma once
#include "stdafx.h"
#include "parser.h"
#include "distcache.h"

class CLIENT
{
public:
	std::string opponent = "any";
	struct PLAYER
	{
		int id;
		int match_wins;
		int elo_points;
		std::string name;
	};
	std::vector<PLAYER> Players;
	PARSER mParser;
	std::stringstream command_buffer;
	DISTCACHE mDistCache;
	enum eUnitCommand {
		CMD_MOVE,
		CMD_ATTACK,
		CMD_SPAWN,
		CMD_ATTACK_MOVE
	};
	struct CMD
	{
		eUnitCommand c;
		POS pos;
		int target_id;
	};
	std::map<int, CMD> mUnitTarget;

	void ParsePlayers(std::vector<std::string> &ServerResponse);

	CLIENT();
	virtual ~CLIENT();
	bool Init(); // connect
	std::string strIPAddress;
	bool bReceivedFirstPing;
	bool LinkDead();

	// Runs the client
	void Run();

	std::string DebugResponse(std::vector<std::string> &text) { return HandleServerResponse(text); }

protected:
	std::string HandleServerResponse(std::vector<std::string> &ServerResponse); // setup parser, call Process, handle mUnitTarget
	void SendMessage( std::string aMessage );

	virtual void Process() = 0;
	virtual void MatchEnd() {}; // reset any data here which is persistent between ticks
	virtual void ConnectionClosed();
	virtual std::string GetPassword() = 0;
	virtual std::string GetPreferredOpponents() = 0;
	virtual bool NeedDebugLog() = 0;
	std::ofstream mDebugLog;
#ifdef WIN32
	SOCKET mConnectionSocket;
#else
	int mConnectionSocket;
#endif

	std::vector<std::pair<UnitType, MAP_OBJECT*>> GetNearObjects(const POS& pos);

	// returns true when it's done
	std::pair<bool, std::string> Move(const std::pair<int, CMD>& cmd);
	std::pair<bool, std::string> Spawn(const std::pair<int, CMD>& cmd);
	std::pair<bool, std::string> Attack(const std::pair<int, CMD>& cmd);
	std::pair<bool, std::string> AttackMove(const std::pair<int, CMD>& cmd);
};

CLIENT *CreateClient();
