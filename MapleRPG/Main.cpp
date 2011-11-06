#pragma region Includes
#include "libircclient.h"
#include "sha1.h"
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
#include <atomic>
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

#pragma region Utility Functions
string passhash(string s) {
	static unsigned char hash[20];
	static char hexstring[41];
	sha1::calc(("91225dcd39598ebb2ac72941e55ec1c94c62bb74"+s).c_str(), s.size() ,hash); // 10 is the length of the string
	sha1::toHexString(hash, hexstring);
	return hexstring;
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
thread* t = nullptr;
mutex mlock;
atomic_bool connected;
map<string, player> players;
#pragma endregion

#pragma region Loading and Saving
void load() {
	ifstream file("players.dat", ios::in);
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
	file.close();
}
void save() {
	ofstream file("players.dat", ios::out);
	int n = count_if(players.begin(), players.end(), [&](pair<const string, player>& it){return !it.second.name.empty();});
	file << n << endl;
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
	file.close();
}
#pragma endregion

#pragma region The loop
void loop() {
	while (connected) {
		this_thread::sleep_for(chrono::seconds(5));
		lock_guard<mutex> l(mlock);
		for (auto it = players.begin(); it != players.end(); it++) {
			player& p = it->second;
			if (p.name.empty()) {
				continue;
			}
		}
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
	connected = true;
	t = new thread(loop);
}
void event_msg(irc_session_t* s, const char* e, const char* origin, const char** params, unsigned int count) {
	lock_guard<mutex> l(mlock);
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
#define reply(x) irc_cmd_msg(s, nick.c_str(), x)
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
			reply("Data saved");
		} else if (c[0] == "load") {
			load();
			reply("Data loaded");
		}
	}
	if (c[0] == "register") {
		cassert(3);
		if (!players[c[1]].name.empty()) {
			reply("That name has already been taken");
			return;
		}
		player p;
		p.name = c[1];
		p.pass = passhash(c[1]+c[2]);
		p.level = 0;
		p.exp = 0;
		players[p.name] = p;
		irc_cmd_msg(s, "#maplerpg", ("Please welcome our new player "+p.name).c_str());
	} else if (c[0] == "login") {
		cassert(3);
		player& p = players[c[1]];
		if (p.name.empty()) {
			reply("That character doesn't exist");
			return;
		}
		if (passhash(c[1]+c[2]) != p.pass) {
			reply("Incorrect password");
			return;
		}
		p.user = nick;
		irc_cmd_msg(s, "#maplerpg", (p.name+" is now signed in from "+p.user).c_str());
	} else if (c[0] == "logout") {
		auto it = find_if(players.begin(), players.end(), [&](pair<const string, player>& it){return it.second.user == nick;});
		if (it != players.end()) {
			player& p = it->second;
			p.user = "";
			irc_cmd_msg(s, "#maplerpg", (p.name+" signed out").c_str());
		}
	} else if (c[0] == "help" or c[0] == "?") {
		if (c.size() == 1) {
			reply("Available commands");
			reply( "register <username> <password>");
			reply("login <username> <password>");
			reply("logout");
			if (nick == "Retep998") {
				reply("slap <name>");
				reply("kick <name>");
				reply("save");
				reply("load");
				reply("exit");
				reply("identify <password>");
			}
		} else {
			reply("I don't know anything more about that command");
		}
	}
}
void event_cmsg(irc_session_t* s, const char* e, const char* origin, const char** params, unsigned int count) {
	lock_guard<mutex> l(mlock);
	string fullnick = origin;
	size_t p = fullnick.find('!');
	string nick = fullnick.substr(0, p);
	string chan = params[0];
	string msg;
	if (count > 1) {
		msg = params[1];
	}
	cout << "[" << chan << "]" << nick << ": " << msg << endl;
	if (chan != "#maplerpg") {
		return;
	}
	auto it = find_if(players.begin(), players.end(), [&](pair<const string, player>& it){return it.second.user == nick;});
	if (it != players.end()) {
		player& p = it->second;
		reply("You died and lost 5% of your exp");
		p.exp = max(0, p.exp-0.05*exptnl[p.level]);
	}
}
void event_nick(irc_session_t* s, const char* e, const char* origin, const char** params, unsigned int count) {
	lock_guard<mutex> l(mlock);
	string fullnick = origin;
	size_t p = fullnick.find('!');
	string nick = fullnick.substr(0, p);
	string newnick = params[0];
	cout << nick << " changed their nickname to " << newnick << endl;
	auto it = find_if(players.begin(), players.end(), [&](pair<const string, player>& it){return it.second.user == nick;});
	if (it != players.end()) {
		player& p = it->second;
		p.user = newnick;
	}
}
void event_leave(irc_session_t* s, const char* e, const char* origin, const char** params, unsigned int count) {
	lock_guard<mutex> l(mlock);
	string fullnick = origin;
	size_t p = fullnick.find('!');
	string nick = fullnick.substr(0, p);
	cout << nick << " has left" << endl;
	auto it = find_if(players.begin(), players.end(), [&](pair<const string, player>& it){return it.second.user == nick;});
	if (it != players.end()) {
		player& p = it->second;
		p.user = "";
		irc_cmd_msg(s, "#maplerpg", (p.name+" signed out").c_str());
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
	callbacks.event_nick = event_nick;
	callbacks.event_quit = event_leave;
	callbacks.event_part = event_leave;
	irc_session_t* s = irc_create_session(&callbacks);
	irc_ctx_t ctx = {"#maplerpg", "MapleBot"};
	irc_set_ctx(s, &ctx);
	irc_connect(s, "irc.fyrechat.net", 6667, 0, "MapleBot", "MapleBot", "MapleBot");
	load();
	irc_run(s);
	connected = false;
	t->join();
	save();
	return 0;
}
#pragma endregion