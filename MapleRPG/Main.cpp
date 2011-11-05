#include "libircclient.h"
#include <iostream>
#include <string>
#include <thread>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <vector>
#include <atomic>
#include <ciso646>
using namespace std;

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
	if (nick == "Retep998") {
		if (c[0] == "identify") {
			irc_cmd_msg(s, "NickServ", ("IDENTIFY "+c[1]).c_str());
		} else if (c[0] == "slap") {
			irc_cmd_me(s, "#maplerpg", ("slaps "+c[1]+" around a bit with a large trout").c_str());
		} else if (c[0] == "exit") {
			irc_cmd_quit(s, "Shutting down...");
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
		irc_cmd_msg(s, chan.c_str(), ("Shut up "+nick+"!").c_str());
	}
}
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
	callbacks.event_channel_notice = event_cmsg;
	irc_session_t* s = irc_create_session(&callbacks);
	irc_ctx_t ctx = {"#maplerpg", "MapleBot"};
	irc_set_ctx(s, &ctx);
	irc_connect(s, "irc.fyrechat.net", 6667, 0, "MapleBot", "MapleBot", "MapleBot");
	irc_run(s);
	return 0;
}