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
	JOIN,// RPLNAM uno alla volta
	QUIT,
	PRIVMSG,
	PART,
	TOPIC,
	NICK,
	NAMES,
	KICK,
	// RPLNAM uno alla volta
	// MODE,
	// KILL,
	// BAN,
	// OPER,
	// ecc..
};

void	initializeMess(Message *mess, std::vector<std::string> v);
void	clearMess(Message *mess);

#endif