#include "stdafx.h"
#include "Client.h"
#include <sstream>
#define SERVER_PORT 4242

CLIENT::CLIENT()
{
#ifdef WIN32
	mConnectionSocket = INVALID_SOCKET;
#else
	mConnectionSocket = -1;
#endif
	mDistCache.LoadFromFile("distcache.bin");
}

CLIENT::~CLIENT()
{
#ifdef WIN32
	if( mConnectionSocket != INVALID_SOCKET )
	{
		closesocket( mConnectionSocket );
	}
	WSACleanup();
#else
	if (mConnectionSocket!=-1)
	{
		close(mConnectionSocket);
	}
#endif
}

bool CLIENT::LinkDead()
{
#ifdef WIN32
	return mConnectionSocket==INVALID_SOCKET;
#else
	return mConnectionSocket == -1;
#endif
}

bool CLIENT::Init()
{
#ifdef WIN32
	static int wsa_startup_done = 0;
	if (!wsa_startup_done)
	{
		wsa_startup_done = 1;
		WSADATA WSAData;

		if( WSAStartup( 0x101,&WSAData ) != 0 )
		{
			std::cout << "Error: Cannot start windows sockets!" << std::endl;
			return false;
		}
	}
#endif
	unsigned long addr = inet_addr( strIPAddress.c_str() );
	sockaddr_in ServerSocketAddress;
	ServerSocketAddress.sin_addr.s_addr = addr;
	ServerSocketAddress.sin_family = AF_INET;
	ServerSocketAddress.sin_port = htons( SERVER_PORT );
	mConnectionSocket = socket( AF_INET, SOCK_STREAM, 0 );
#ifdef WIN32
	if( mConnectionSocket == INVALID_SOCKET )
#else
	if (mConnectionSocket == -1 )
#endif
	{
		std::cout << "Error: Cannot open a socket!" << std::endl;
		return false;
	}
	if ( connect( mConnectionSocket,(struct sockaddr*)&ServerSocketAddress, sizeof( ServerSocketAddress ) ) )
	{
		std::cout << "Error: Cannot connect to " << strIPAddress << "!" << std::endl;
#ifdef WIN32
		closesocket( mConnectionSocket );
#else
		close( mConnectionSocket );
#endif
		return false;
	}
	SendMessage("login " + GetPassword());
	bReceivedFirstPing = false;
	return true;
}

void CLIENT::ConnectionClosed()
{
	std::cout<<"Connection closed"<<std::endl;
#ifdef WIN32
	mConnectionSocket = INVALID_SOCKET;
#else
	mConnectionSocket = -1;
#endif
}

void CLIENT::SendMessage( std::string aMessage )
{
	if (LinkDead()) return;
	if (aMessage.length()==0) return;
	if (aMessage[aMessage.length()-1]!='\n') aMessage+="\n";
	if (NeedDebugLog() && mDebugLog.is_open())
	{
		mDebugLog<<"Sent: "<<aMessage;
	}
	int SentBytes = send( mConnectionSocket, aMessage.c_str(), int(aMessage.size()), 0 );
	if (SentBytes!=aMessage.size())
	{
#ifdef WIN32
		closesocket( mConnectionSocket );
#else
		close( mConnectionSocket );
#endif
		ConnectionClosed();
	}
}

void CLIENT::ParsePlayers(std::vector<std::string> &ServerResponse)
{
	Players.clear();
	if (ServerResponse[0].substr(0, 7)=="players")
	{
		int count = atoi(ServerResponse[0].substr(8).c_str());
		Players.resize(count);
		int r;
		for(r=0;r<count;r++)
		{
			std::stringstream ss;
			ss<<ServerResponse[r+1];
			ss>>Players[r].id;
			ss>>Players[r].match_wins;
			ss>>Players[r].elo_points;
			char str[31];
			ss.getline(str, 30);
			str[30]=0;
			Players[r].name = str;
		}
	}
}

void CLIENT::Run()
{
	if (NeedDebugLog())
	{
		mDebugLog.open("debug.log", std::ofstream::out | std::ofstream::app);
	}

	std::string strLastLineRemaining;
	std::vector<std::string> LastServerResponse;
	int last_connect_try_time = GetTickCount();
	for(;;)
	{
		if (LinkDead())
		{
			LastServerResponse.clear();
			strLastLineRemaining = "";
			Init();
		}
		if (LinkDead())
		{
#ifdef WIN32
			Sleep(1000);
#else
			sleep(1);
#endif
			continue;
		}
		const size_t ReceiveBufferSize = 1<<16;
		char ReceiveBuffer[ ReceiveBufferSize+1 ] = {0};

		int ReceivedBytesCount = recv( mConnectionSocket, ReceiveBuffer, ReceiveBufferSize, 0 );

		if( ReceivedBytesCount == 0 || ReceivedBytesCount == -1)
		{
			// connection is closed or failed
#ifdef WIN32
			closesocket( mConnectionSocket );
#else
			close( mConnectionSocket );
#endif
			ConnectionClosed();
			continue;
		}
		ReceiveBuffer[ReceivedBytesCount]=0;
		char *line_start = ReceiveBuffer;
		for(;;)
		{
			char *s = strchr(line_start, '\n');
			if (!s)
			{
				strLastLineRemaining = line_start;
				break;
			} else
			{
				std::string alma=strLastLineRemaining;
				*s=0;
				alma+=line_start;
				line_start = s+1;
				strLastLineRemaining = "";
				if (alma=="fail")
				{
					std::cout<<"Login failed :("<<std::endl;
				} else
				if (alma=="ping")
				{
					SendMessage(std::string("pong"));
					if (!bReceivedFirstPing)
					{
						std::cout<<"Login OK"<<std::endl;
						SendMessage(std::string("opponent ")+GetPreferredOpponents());
						bReceivedFirstPing = true;
					} else
					{
						time_t tt;
						time(&tt);
						struct tm *tm = localtime(&tt);
						char str[20];
						sprintf(str, "%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
						std::cout<<"PING "<<str<<std::endl;
					}
				} else
				{
					LastServerResponse.push_back(alma);

					if (alma==".")
					{
						if (LastServerResponse.front().substr(0, 7)=="players")
						{
							ParsePlayers(LastServerResponse);
						} else
						{
							if (NeedDebugLog() && mDebugLog.is_open())
							{
								for(unsigned int i=0;i<LastServerResponse.size();i++)
									mDebugLog<<LastServerResponse[i]<<std::endl;
							}
							std::string strResponse = HandleServerResponse(LastServerResponse);
							if (!strResponse.empty())
							{
								SendMessage(strResponse);
							}
						}
						LastServerResponse.clear();
					}
				}
			}
		}
	}
}

std::string CLIENT::HandleServerResponse(std::vector<std::string> &ServerResponse)
{
	mParser.Parse(ServerResponse);
	if (mParser.w!=0 && mDistCache.mDistMap.empty())
	{
		mDistCache.CreateFromParser(mParser);
		mDistCache.SaveToFile("distcache.bin");
	}
	std::stringstream ss;
	if (mParser.match_result==PARSER::ONGOING)
	{
		ss << "tick "<<mParser.tick<<"\n";
		Process();
		ss<<command_buffer.str();
		for(std::map<int, CLIENT::CMD>::iterator it=mUnitTarget.begin();it!=mUnitTarget.end();)
		{
			bool cmd_done = false;

			MAP_OBJECT *q = NULL;
			for(std::vector<MAP_OBJECT>::iterator p=mParser.Units.begin(); p!=mParser.Units.end();p++) { if (p->id==it->first) { q=&*p; break; } }
			if (q==NULL)
			{
				cmd_done = true;
			} else if (it->second.c == CLIENT::CMD_MOVE) {
				auto p = Move(*it);
				cmd_done = p.first;
				ss << p.second;
			} else if (it->second.c == CLIENT::CMD_SPAWN) {
				auto p = Spawn(*it);
				cmd_done = p.first;
				ss << p.second;
			} else if (it->second.c == CLIENT::CMD_ATTACK) {
				auto p = Attack(*it);
				cmd_done = p.first;
				ss << p.second;
			}
			std::map<int, CLIENT::CMD>::iterator it2=it;
			it2++;
			if (cmd_done)
			{
				mUnitTarget.erase(it);
			}
			it=it2;
		}
		command_buffer.str(std::string());
		ss<<".";
	} else
	{
		mUnitTarget.clear();
		MatchEnd();
		ss<<".";
	}
	return ss.str();
}




int main(int argc, char* argv[])
{
	std::cout.sync_with_stdio(false);
	std::string server_address;
	server_address = "172.22.22.173";
	std::cout<<"using default server address: " + server_address <<std::endl;
	CLIENT *pClient = CreateClient();

	if (argc > 1) {
		pClient->opponent = argv[1];
	}

	/* for debugging:  */
	std::ifstream debug_file("test.txt");
	if (debug_file.is_open())
	{
		std::string line;
		std::vector<std::string> full;
		while (std::getline(debug_file, line))
		{
			full.push_back(line);
		}
		std::string resp = pClient->DebugResponse(full);
		std::cout<<"response: "<<resp <<std::endl;
	}
	/**/

	pClient->strIPAddress = server_address;
	if (!pClient->Init())
	{
		std::cout<<"Connection failed"<<std::endl;
	} else
	{
		pClient->Run();
	}
	delete pClient;
	return 0;
}

std::pair<bool, std::string> CLIENT::Move(const std::pair<int, CMD>& cmd) {
	MAP_OBJECT *q = mParser.FindUnit(cmd.first);

	if (!q) {
		return {true, ""};
	}
	if (q->pos == cmd.second.pos) {
		return {true, ""};
	} else {
		POS t = mDistCache.GetNextTowards(q->pos, cmd.second.pos);
		if (!t.IsValid()) {
			return {true, ""};
		} else {
			std::stringstream ss;
			ss << "queen_move " << cmd.first << " " << t.x << " " << t.y << "\n";
			return {t == cmd.second.pos, ss.str()};
		}
	}
}

std::pair<bool, std::string> CLIENT::Spawn(const std::pair<int, CMD>& cmd) {
	MAP_OBJECT *q = mParser.FindUnit(cmd.first);
	if (!q) {
		return {true, ""};
	}

	bool do_spawn = false;
	if (q->pos == cmd.second.pos) {
		do_spawn = true;
	} else {
		POS t = mDistCache.GetNextTowards(q->pos, cmd.second.pos);
		if (!t.IsValid()) {
			return {true, ""};
		} else if (t==cmd.second.pos) {
			do_spawn = true;
		} else {
			std::stringstream ss;
			ss<<"queen_move "<<cmd.first<<" "<<t.x<<" "<<t.y<<"\n";
			return {false, ss.str()};
		}
	}
	if (do_spawn) {
		std::stringstream ss;
		ss<<"queen_spawn "<<cmd.first<<" "<<cmd.second.pos.x<<" "<<cmd.second.pos.y<<"\n";
		return {true, ss.str()};
	}
	return {true, ""};
}

std::pair<bool, std::string> CLIENT::Attack(const std::pair<int, CMD>& cmd) {
	MAP_OBJECT *q = mParser.FindUnit(cmd.first);
	if (!q) {
		return {true, ""};
	}

	MAP_OBJECT *t = NULL;
	POS t_pos;
	std::vector<MAP_OBJECT>::iterator p;
	for(p=mParser.Units.begin(); p!=mParser.Units.end();p++) { if (p->id==cmd.second.target_id) { t=&*p; t_pos = p->pos; break; } }
	if (t==NULL) for(p=mParser.CreepTumors.begin();p!=mParser.CreepTumors.end();p++) { if (p->id==cmd.second.target_id) { t=&*p; t_pos=p->pos; break; } }
	if (t==NULL)
	{
		if (mParser.EnemyHatchery.id==cmd.second.target_id)
		{
			t=&mParser.EnemyHatchery;
			t_pos=mParser.EnemyHatchery.pos;
			// find closest point of hatchery:
			int i;
			for (i = 0; i<HATCHERY_SIZE - 1; i++) if (q->pos.y>t_pos.y) t_pos.y++;
			for (i = 0; i<HATCHERY_SIZE - 1; i++) if (q->pos.x>t_pos.x) t_pos.x++;
		}
	}
	if (t==NULL) {
		return {true, ""};
	} else {
		bool do_attack = false;
		if (q->pos==t_pos) do_attack = true;
		else
		{
			POS next_pos = mDistCache.GetNextTowards(q->pos, t_pos);
			if (!next_pos.IsValid()) {
				return {true,  ""};
			} else if (next_pos==t_pos) {
				do_attack = true;
			} else {
				std::stringstream ss;
				ss<<"queen_move "<<cmd.first<<" "<<next_pos.x<<" "<<next_pos.y<<"\n";
				return {false, ss.str()};
			}
		}
		if (do_attack) {
			std::stringstream ss;
			ss<<"queen_attack "<<cmd.first<<" "<<cmd.second.target_id<<"\n";
			return {false, ss.str()};
		}
	}
	return {true, ""};
}

std::pair<bool, std::string> CLIENT::AttackMove(const std::pair<int, CMD>& cmd) {
	MAP_OBJECT *q = mParser.FindUnit(cmd.first);
	if (!q) {
		return {true, ""};
	}

	MAP_OBJECT *t = mParser.FindUnit(cmd.second.target_id);
	if (!t) {
		return {true, ""};
	}

	if (q->pos.IsNear(t->pos)) {
		auto new_cmd = cmd;
		new_cmd.second.c = CLIENT::CMD_ATTACK;
		return Attack(new_cmd);
	}

	std::vector<std::pair<UnitType, MAP_OBJECT*>> near_objects;
	near_objects.push_back(mParser.GetUnitAt(q->pos.ShiftDir(POS::SHIFT_UP)));
	near_objects.push_back(mParser.GetUnitAt(q->pos.ShiftDir(POS::SHIFT_DOWN)));
	near_objects.push_back(mParser.GetUnitAt(q->pos.ShiftDir(POS::SHIFT_LEFT)));
	near_objects.push_back(mParser.GetUnitAt(q->pos.ShiftDir(POS::SHIFT_RIGHT)));

	for (auto& p : near_objects) {
		if (!p.second) {
			continue;
		}
		switch (p.first) {
			default:
				break;
			case UnitType::kEnemyQueen:
			case UnitType::kEnemyCreepTumor:
			case UnitType::kEnemyHatchery:
				auto new_cmd = cmd;
				new_cmd.second.c = CLIENT::CMD_ATTACK;
				new_cmd.second.target_id = p.second->id;
				return Attack(new_cmd);
		}
	}

	auto new_cmd = cmd;
	new_cmd.second.c = CLIENT::CMD_MOVE;
	new_cmd.second.pos = t->pos;
	return Move(cmd);
}
