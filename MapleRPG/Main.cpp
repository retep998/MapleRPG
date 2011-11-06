#pragma region Includes
#include "libircclient.h"
#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>
#include <ciso646>
#include <map>
#include <algorithm>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std;
#pragma endregion

#pragma region Constants
const int exptnl[199] = {
	15, 34, 57, 92, 135, 372, 560, 840, 1242, 1716,
	2360, 3216, 4200, 5460, 7050, 8840, 11040, 13716, 16680, 20216,
	24402, 28980, 34320, 40512, 47216, 54900, 63666, 73080, 83720, 95700,
	108480, 122760, 138666, 155540, 174216, 194832, 216600, 240500, 266682, 294216,
	324240, 356916, 391160, 428280, 468450, 510420, 555680, 604416, 655200, 709716,
	748608, 789631, 832902, 878545, 926689, 977471, 1031036, 1087536, 1147132, 1209994,
	1276301, 1346242, 1420016, 1497832, 1579913, 1666492, 1757815, 1854143, 1955750, 2062925,
	2175973, 2295216, 2420993, 2553663, 2693603, 2841212, 2996910, 3161140, 3334370, 3517093,
	3709829, 3913127, 4127566, 4353756, 4592341, 4844001, 5109452, 5389449, 5684790, 5996316,
	6324914, 6671519, 7037118, 7422752, 7829518, 8258575, 8711144, 9188514, 9692044, 10223168,
	10783397, 11374327, 11997640, 12655110, 13348610, 14080113, 14851703, 15665576, 16524049, 17429566,
	18384706, 19392187, 20454878, 21575805, 22758159, 24005306, 25320796, 26708375, 28171993, 29715818,
	31344244, 33061908, 34873700, 36784778, 38800583, 40926854, 43169645, 45535341, 48030677, 50662758,
	53439077, 56367538, 59456479, 62714694, 66151459, 69776558, 73600313, 77633610, 81887931, 86375389,
	91108760, 96101520, 101367883, 106992842, 112782213, 118962678, 125481832, 132358236, 139611467, 147262175,
	155332142, 163844343, 172823012, 182293713, 192283408, 202820538, 213935103, 225658746, 238024845, 251068606,
	264827165, 279339639, 294647508, 310794191, 327825712, 345790561, 364739883, 384727628, 405810702, 428049128,
	451506220, 476248760, 502347192, 529875818, 558913012, 589541445, 621848316, 655925603, 691870326, 729784819,
	769777027, 811960808, 856456260, 903390063, 952895838, 1005114529, 1060194805, 1118293480, 1179575962, 1244216724,
	1312399800, 1384319309, 1460180007, 1540197871, 1624600714, 1713628833, 1807535693, 1906558648, 2011069705
};
#pragma endregion

#pragma region MapleStructures
struct player {
	string name;
	string pass;
	string user;
	int level;
	int exp;
	int hp, mp;
	int astr, adex, aint, aluk;
	int curmap;
};
struct item {

};
#pragma endregion

#pragma region MapleData
map<string, player> players;
#pragma endregion

#pragma region Loading and Saving
void load() {
	ifstream file("data");
	if (!file) {
		return;
	}
	int n;
	file >> n;
	for (int i = 0; i < n; i++) {
		player p;
		file >> p.name;
		file >> p.pass;
		file >> p.level;
		file >> p.exp;
		players[p.name] = p;
	}
}
void save() {
	ofstream file("data");
	int n = count_if(players.begin(), players.end(), [&](pair<const string, player>& it){return !it.second.name.empty();});
	for (auto it = players.begin(); it != players.end(); it++) {
		player& p = it->second;
		if (p.name.empty()) {
			continue;
		}
		file << p.name << endl;
		file << p.pass << endl;
		file << p.level << endl;
		file << p.exp << endl;
	}
}
#pragma endregion

#pragma region IRCCrap
struct irc_ctx_t {
	char* channel;
	char* nick;
};
void event_connect(irc_session_t* s, const char* e, const char* origin, const char** params, unsigned int count) {
	irc_ctx_t* ctx = (irc_ctx_t*)irc_get_ctx(s);
	irc_cmd_join(s, ctx->channel, 0);
}
void event_msg(irc_session_t* s, const char* e, const char* origin, const char** params, unsigned int count) {
	string fullnick = origin;
	size_t p = fullnick.find('!');
	string nick = fullnick.substr(0, p);
	string me = params[0];
	string msg;
	if (count > 1) {
		msg = params[1];
	}
	cout << "[msg to " << me << "]" << nick << ": " << msg << endl;
	istringstream iss(msg);
	vector<string> c;
	copy(istream_iterator<string>(iss), istream_iterator<string>(), back_inserter<vector<string>>(c));
#define cassert(n) if (c.size() < n) {irc_cmd_msg(s, nick.c_str(), "Invalid number of arguments"); return;}
	cassert(1);
	if (nick == "Retep998") {
		if (c[0] == "identify") {
			cassert(2);
			irc_cmd_msg(s, "NickServ", ("IDENTIFY "+c[1]).c_str());
		} else if (c[0] == "slap") {
			cassert(2);
			irc_cmd_me(s, "#maplerpg", ("slaps "+c[1]+" around a bit with a large trout").c_str());
		} else if (c[0] == "kick") {
			cassert(2);
			irc_cmd_kick(s, c[1].c_str(), "#maplerpg", "Requested by an administrator");
		} else if (c[0] == "exit") {
			irc_cmd_quit(s, "Shutting down...");
		} else if (c[0] == "save") {
			save();
			irc_cmd_msg(s, "Retep998", "Data saved");
		} else if (c[0] == "load") {
			load();
			irc_cmd_msg(s, "Retep998", "Data loaded");
		}
	}
	if (c[0] == "register") {
		cassert(3);
		if (!players[c[1]].name.empty()) {
			irc_cmd_msg(s, nick.c_str(), "That name has already been taken");
			return;
		}
		player p;
		p.name = c[1];
		p.pass = c[2];
		p.level = 0;
		p.exp = 0;
		players[p.name] = p;
		irc_cmd_msg(s, "#maplerpg", ("Please welcome our new player "+p.name).c_str());
	} else if (c[0] == "help" or c[0] == "?") {
		if (c.size() == 1) {
			irc_cmd_msg(s, nick.c_str(), "Available commands");
			irc_cmd_msg(s, nick.c_str(), "register <username> <password>");
			if (nick == "Retep998") {
				irc_cmd_msg(s, nick.c_str(), "slap <name>");
				irc_cmd_msg(s, nick.c_str(), "kick <name>");
				irc_cmd_msg(s, nick.c_str(), "save");
				irc_cmd_msg(s, nick.c_str(), "load");
				irc_cmd_msg(s, nick.c_str(), "exit");
				irc_cmd_msg(s, nick.c_str(), "identify <password>");
			}
		} else {
			irc_cmd_msg(s, nick.c_str(), "I don't know anything more about that command");
		}
	}
}
void event_cmsg(irc_session_t* s, const char* e, const char* origin, const char** params, unsigned int count) {
	string fullnick = origin;
	size_t p = fullnick.find('!');
	string nick = fullnick.substr(0, p);
	string chan = params[0];
	string msg;
	if (count > 1) {
		msg = params[1];
	}
	cout << "[" << chan << "]" << nick << ": " << msg << endl;
	if (nick != "Retep998" or chan != "#maplerpg") {
		auto it = find_if(players.begin(), players.end(), [&](pair<const string, player>& it){return it.second.user == nick;});
		if (it != players.end()) {
			irc_cmd_msg(s, chan.c_str(), (nick+" has died and lost 5% of their exp").c_str());
			it->second.exp = max(0, it->second.exp-0.05*exptnl[it->second.level]);
		}
	}
}
#pragma endregion

#pragma region Main
int main() {
#ifdef WIN32
	WSADATA d;
	WSAStartup(MAKEWORD(2, 2), &d);
#else
#endif
	irc_callbacks_t	callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.event_connect = event_connect;
	callbacks.event_notice = event_msg;
	callbacks.event_privmsg = event_msg;
	callbacks.event_channel = event_cmsg;
	irc_session_t* s = irc_create_session(&callbacks);
	irc_ctx_t ctx = {"#maplerpg", "MapleBot"};
	irc_set_ctx(s, &ctx);
	irc_connect(s, "irc.fyrechat.net", 6667, 0, "MapleBot", "MapleBot", "MapleBot");
	load();
	irc_run(s);
	save();
	return 0;
}
#pragma endregion