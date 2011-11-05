#include "libircclient.h"
#include <iostream>
#include <string>
#include <thread>
#include <cstdint>
#include <fstream>
using namespace std;

struct irc_ctx_t {
	char* channel;
	char* nick;
};
void event_connect(irc_session_t* session, const char* event, const char* origin, const char** params, unsigned int count) {
	irc_ctx_t* ctx = (irc_ctx_t*)irc_get_ctx(session);
	irc_cmd_join(session, ctx->channel, 0);
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
	irc_session_t* s = irc_create_session(&callbacks);
	irc_ctx_t ctx = {"#maplerpg", "MapleBot"};
	irc_set_ctx(s, &ctx);
	irc_connect(s, "irc.fyrechat.net", 6667, 0, "MapleBot", "MapleBot", "MapleBot");
	irc_run(s);
	return 0;
}