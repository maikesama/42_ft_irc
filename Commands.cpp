#include "Commands.hpp"

void	initializeMess(Message *mess, std::vector<std::string> v)
{
	mess->command = v[0];
	for (int i = 1; i < v.size(); i++)
	{
		mess->params.push_back(v[i]);
	}
}

void	clearMess(Message *mess)
{
	mess->command.clear();
	mess->params.clear();
}
