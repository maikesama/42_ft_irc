#ifndef COMMANDS_HPP
#define COMMANDS_HPP

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
	
};

void	initializeMess(Message *mess, std::vector<std::string> v);
void	clearMess(Message *mess);

#endif