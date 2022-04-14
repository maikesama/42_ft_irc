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
	PART,
	TOPIC,
	NICK,
	NAMES,
	LIST,
	MODE, // ADD kick instead of part on the ban
	OPER,
	INVITE,
	KICK,
	// KILL,
	// ecc..
};

void	initializeMess(Message *mess, std::vector<std::string> v);
void	clearMess(Message *mess);

#endif