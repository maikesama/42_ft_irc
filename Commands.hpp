#ifndef COMMANDS_HPP
#define COMMANDS_HPP

class Server;
#include "Server.hpp"
#include <iostream>
#include <vector>

struct Message
{
	std::string				command;
	std::vector<std::string> params;
};


enum ecommands{
	PING,
	JOIN,
	QUIT,
	PRIVMSG,
	NOTICE,
	PART,
	TOPIC,
	NICK,
	NAMES,
	LIST,
	MODE,
	OPER,
	INVITE,
	KICK,
	KILL,
	WHO,
	INFO,
};

void	initializeMess(Message *mess, std::vector<std::string> v);
void	clearMess(Message *mess);

#endif