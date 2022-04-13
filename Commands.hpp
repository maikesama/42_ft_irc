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
	JOIN,//control on mode
	QUIT,
	PRIVMSG,
	PART,
	TOPIC,
	NICK,
	NAMES,// control if secret
	LIST,
	// MODE,
	// KILL,
	// BAN,
	// KICK,
	// OPER,
	// ecc..
};

void	initializeMess(Message *mess, std::vector<std::string> v);
void	clearMess(Message *mess);

#endif