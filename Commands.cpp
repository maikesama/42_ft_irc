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

void	Server::whoCmd(Message *mess, Client *c)
{
	std::string s;
	if (mess->params.size() == 0 || (mess->params.size() == 1 && (!mess->params[0].compare("*") || !mess->params[0].compare("0"))))
	{
		for (int i = 0; i < _cVec.size(); i++)
		{
			if (_cVec[i]->getFd() != c->getFd() && _cVec[i]->haveChannelInCommon(c) == false)
			{
				s = ":42IRC 352 " + c->getNick() + " * " + _cVec[i]->getUsername() + " " + _cVec[i]->getHostAddress() + " 42IRC " + _cVec[i]->getNick() + (isServerOp(_cVec[i]->getFd()) == true ? " H* " : " H ") + " :0 undefined\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
				s.clear();				
			}
		}
		s = ":42IRC 315 " + c->getNick() + " * :End of WHO list\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	else if (mess->params.size() == 1 && mess->params[0][0] == '#' && channelExist(mess->params[0]) == true)
	{
		Channel *ch = findChannel(mess->params[0]);
		for (std::vector<int>::const_iterator it = ch->getClients().begin(); it != ch->getClients().end(); it++)
		{
			if (*it != c->getFd())
			{
				Client *cl = findClient(*it);
				s = ":42IRC 352 " + c->getNick() + " * " + cl->getUsername() + " " + cl->getHostAddress() + " 42IRC " + cl->getNick() + (isServerOp(cl->getFd()) == true ? " H*" : " H") + (ch->isAnOperator(cl->getFd()) ? "@" : "") + (ch->canITalk(c->getFd()) ? "+" : "") + " :0 undefined\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
				s.clear();
			}
		}
		s = ":42IRC 315 " + c->getNick() + " " + ch->getName() +" :End of WHO list\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	else if (mess->params.size() == 1 && findClient(mess->params[0]) != NULL)
	{
		Client *cl = findClient(mess->params[0]);
		s = ":42IRC 352 " + c->getNick() + " * " + cl->getUsername() + " " + cl->getHostAddress() + " 42IRC " + cl->getNick() + (isServerOp(cl->getFd()) == true ? " H*" : " H") + " :0 undefined\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		s.clear();
		s = ":42IRC 315 " + c->getNick() + " " + cl->getNick() +" :End of WHO list\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	s = ":42IRC 315 " + c->getNick() + " * :End of WHO list\r\n";
	send(c->getFd(), s.c_str(), s.size(), 0);
}

void	Server::infoCmd(Message *mess, Client *c)
{
	std::string s = ":42IRC 371 " + c->getNick() + " :Server 42IRC version 1.2, by Michele Paci and Marcello Crisari\r\n";
	s += "42IRC 374 " + c->getNick() +" :End of INFO list";
	send(c->getFd(), s.c_str(), s.size(), 0);
}

void	Server::killCmd(Message *mess, Client *c, fd_set *currentsockets)
{
	std::string ss;
	
	if(mess->params.size() < 2)
	{
		ss = ":42IRC 461 " + c->getNick() + " KILL :Not enough parameters\r\n";
		send(c->getFd(), ss.c_str(), ss.size(), 0);
		return;
	}
	std::string target = mess->params[0];
	std::string reason;
	for (int i = 1; i < mess->params.size(); i++)
	{
		if (i + 1 < mess->params.size())
			reason += mess->params[i] + " ";
		else
			reason += mess->params[i];
	}
	if (isServerOp(c->getFd()) == false)
	{
		ss = ":42IRC 481 " + c->getNick() + " :Permission Denied- You're not an IRC operator\r\n";
		send(c->getFd(), ss.c_str(), ss.size(), 0);
		return;
	}
	if (c->getHostAddress().compare(target) == 0)
	{
		ss = ":42IRC 483 " + c->getNick() + " :You cant kill a server!\r\n";
		send(c->getFd(), ss.c_str(), ss.size(), 0);
		return;
	}
	if (findClient(target) == NULL)
	{
		ss = ":42IRC 401 " + c->getNick() + " " + target + " :No such nick\r\n";
		send(c->getFd(), ss.c_str(), ss.size(), 0);
		return;
	}
	Client *ctk = findClient(target);
	ss = ":" + c->getFullIdentifier() + " KILL " + ctk->getNick() +" " + reason + "\r\n";
	send(ctk->getFd(), ss.c_str(), ss.size(), 0);
	ss.clear();
	ss = ":" + ctk->getFullIdentifier() + " QUIT :Closing link: 42IRC KIlled by " + c->getNick() + " " + reason + "\r\n";
	for (std::vector<std::string>::const_iterator it = ctk->getClientChannel().begin(); it != ctk->getClientChannel().end(); it++)
	{
		Channel *ch = findChannel(*it);
		broadcastToChan(ch, ss, ctk, true);
		ch->removeClient(ctk->getFd());
		if (ch->isAnOperator(ctk->getFd()))
			ch->removeOperator(ctk->getFd());
		if (!ch->getClients().size())
			deleteChannel(ch->getName());
	}
	ss.clear();
	ss = ss = ":" + ctk->getFullIdentifier() + " ERROR :Closing link: 42IRC KIlled by " + c->getNick() + " " + reason + "\r\n";
	send(ctk->getFd(), ss.c_str(), ss.size(), 0);
	closeClientConnection(ctk->getFd(), currentsockets);
}

void	Server::kickCmd(Message *mess, Client *c)
{
	std::string s;
	if (mess->params.size() < 2)
	{
		s = ":42IRC 461 INVITE :Not enough parameters\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	std::string	chan = mess->params[0];
	std::string	ut = mess->params[1];
	std::vector<std::string> user = ft_split((char*)ut.c_str(), ",");
	if (findChannel(chan) == NULL)
	{
		s = ":42IRC 403 " + c->getNick() + " " + chan + "\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	if (c->isOnChannel(chan) == false)
	{
		s = ":42IRC 442 "+ chan + " :You're not on that channel\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	std::string	reason;
	if (mess->params.size() > 2)
	{
		for (int i = 2; i < mess->params.size(); i++)
		{
			if (i + 1 < mess->params.size())
				reason += mess->params[i] + " ";
			else
				reason += mess->params[i];
		}
	}
	Channel *ch = findChannel(chan);
	if (ch->isAnOperator(c->getFd()) == false && isServerOp(c->getFd()) == false)
	{
		s = ":42IRC 482 " + c->getNick() + " " + ch->getName() + " :You're not an operator\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}

	for (std::vector<std::string>::iterator it = user.begin(); it != user.end(); it++)
	{
		Client *ctk = findClient(*it);
		if (ctk != NULL && ctk->isOnChannel(chan) == true)
		{
			s = ":" + c->getFullIdentifier() + " KICK " + chan + " " + *it + " " + (reason.size() > 0 ? reason : ":We don't like you") + "\r\n";
			broadcastToChan(ch, s, c, false);
			ctk->removeChannel(chan);
			ch->removeClient(ctk->getFd());
			if (ch->isAnOperator(ctk->getFd()))
				ch->removeOperator(ctk->getFd());
		}
		else
		{
			s = ":42IRC 441 " + c->getNick() + " " + *it + " " + ch->getName() + " :They aren't on that channel\r\n";
			send(c->getFd(), s.c_str(), s.size(), 0);
		}
	}
}

void	Server::inviteCmd(Message *mess, Client *c)
{
	std::string s;
	if (mess->params.size() < 2)
	{
		s = ":42IRC 461 INVITE :Not enough parameters\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	std::string	nick = mess->params[0];
	if (findClient(nick) == NULL)
	{
		s = ":42IRC 401 " + c->getNick() + " " + nick + " :No such nick\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	std::string	chan = mess->params[1];
	if (findChannel(chan) == NULL)
	{
		s = ":42IRC 403 " + c->getNick() + " " + chan + "\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	if (c->isOnChannel(chan) == false)
	{
		s = ":42IRC 442 "+ chan + " :You're not on that channel\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}

	Client *cta = findClient(nick);
	if (cta->isOnChannel(chan) == true)
	{
		s = ":42IRC 443 " + c->getNick() + " " + nick + " " + chan + " :is already on channel\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}

	s = "42IRC 341 " + c->getNick() + " " + nick + " " + chan + "\r\n";
	s += ":" + c->getFullIdentifier() + " INVITE " + nick + " " + chan + "\r\n";
	send(cta->getFd(), s.c_str(), s.size(), 0);
}

void	Server::modeCmd(Message *mess, Client *c)
{
	std::string s;
	std::string target = mess->params[0];

	//check if target exist
	if (target[0] == '#' && channelExist(target) == false)
	{
		s = ":42IRC 403 " + c->getNick() + target + "\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	else if (findClient(target) == NULL && channelExist(target) == false)
	{
		s = ":42IRC 401 " + c->getNick() + " " +target + " :No such nick/channel\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}

	//display modes for channel/user
	if (mess->params.size() == 1)
	{
		if (target[0] == '#')
		{
			Channel *ch = findChannel(target);
			s = ":42IRC 324 " + c->getNick() + " " + target + " " + ch->getModes() + "\r\n";
			send(c->getFd(), s.c_str(), s.size(), 0);
		}
		else
		{
			Client *c = findClient(target);
			s = ":42IRC 221 " + c->getNick() + (isServerOp(c->getFd()) ?  " o\r\n" : "\r\n");
		}
	}

	//set modes
	if (mess->params.size() >= 2)
	{
		std::string modeToAdd = mess->params[1];
		if (modeToAdd[0] != '+' && modeToAdd[0] != '-')
			return ;
		std::string sign = modeToAdd.substr(0, 1);
		modeToAdd = modeToAdd.substr(1, std::string::npos);

		if (target[0] == '#')
		{
			Channel *ch = findChannel(target);
			std::vector<std::string> modeParams;
			for (int i = 2; i < mess->params.size(); i++)
				modeParams.push_back(mess->params[i]);

			int	paramsIt = 0;
			for (int i = 0; i < modeToAdd.size(); i++)
			{
				if (ch->isAnOperator(c->getFd()) == true || isServerOp(c->getFd()) == true)
					switchMode(modeToAdd[i], sign, modeParams, &paramsIt, ch, c);
				else
				{
					s = ":42IRC 482 " + c->getNick() + " " + ch->getName() + " :You're not an operator\r\n";
					send(c->getFd(), s.c_str(), s.size(), 0);
					return ;
				}
			}
		}
		else
		{
			Client *c = findClient(target);
			if (!sign.compare("-") && !modeToAdd.compare("o"))
			{
				if (isServerOp(c->getFd()))
				{
					removeServerOp(c->getFd());
					s = ":" + c->getFullIdentifier() + " MODE " + c->getNick() + " :" + sign + modeToAdd[0] + "\r\n";
					send(c->getFd(), s.c_str(), s.size(), 0);
				}
			}
			else
			{
				s = ":42IRC 501 " + c->getNick() + " :Unkown MODE flag\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
			}
		}
	}
}

void	Server::switchMode(char m, std::string sign, std::vector<std::string> mp, int* pi, Channel *ch, Client *c)
{
	std::string s;

	switch (m)
	{
		case 'm' : case 'n' : case 's' : case 't' : 
		ch->setModes(sign + m);
		s = ":" + c->getFullIdentifier() + " MODE " + ch->getName() + " :" + sign + m + "\r\n";
		broadcastToChan(ch, s, c, false);
		break;

		case 'v' :
			if (*pi < mp.size())
			{
				Client *cto = findClient(mp[*pi]);
				if (cto == NULL || cto->isOnChannel(ch->getName()) == false)
				{
					s = ":42IRC 441 " + c->getNick() + " " + mp[*pi] + " " + ch->getName() + " :They aren't on that channel\r\n";
					send(c->getFd(), s.c_str(), s.size(), 0);
					*pi += 1;
					return;
				}
				else if (cto != NULL)
				{
					!sign.compare("+") ? ch->CanTalk(cto->getFd()) : ch->CantTalk(cto->getFd());
					s = ":" + c->getFullIdentifier() + " MODE " + ch->getName() + " :" + sign + m + " " + cto->getNick() +"\r\n";
					broadcastToChan(ch, s, c, false);
				}
				*pi += 1;
			}
			else
			{
				s = ":42IRC 461 MODE :Not enough parameters\r\n"; send(c->getFd(), s.c_str(), s.size(), 0);
			}
			break ;

		case 'o' :
			if (*pi < mp.size())
			{
				Client *cto = findClient(mp[*pi]);

				if (cto == NULL || cto->isOnChannel(ch->getName()) == false)
				{
					s = ":42IRC 441 " + c->getNick() + " " + mp[*pi] + " " + ch->getName() + " :They aren't on that channel\r\n";
					send(c->getFd(), s.c_str(), s.size(), 0);
					*pi += 1;
					return;
				}
				else if (cto != NULL)
				{
					!sign.compare("+") ? ch->setNewOperator(cto->getFd()) : ch->removeOperator(cto->getFd());
					s = ":" + c->getFullIdentifier() + " MODE " + ch->getName() + " :" + sign + m + " " + cto->getNick() +"\r\n";
					broadcastToChan(ch, s, c, false);
				}
				*pi += 1;
			}
			else
			{
				s = ":42IRC 461 MODE :Not enough parameters\r\n"; send(c->getFd(), s.c_str(), s.size(), 0);
			}
			break ;

		case 'k' :
			if (*pi < mp.size() && !sign.compare("+"))
			{
				ch->setKey(mp[*pi]);
				*pi += 1;
				ch->setModes(sign + m);
				s = ":" + c->getFullIdentifier() + " MODE " + ch->getName() + " :" + sign + m + " " + mp[*pi] + "\r\n";
				broadcastToChan(ch, s, c, false);
			}
			else if (!sign.compare("-"))
			{
				ch->setKey("");
				ch->setModes(sign + m);
				s = ":" + c->getFullIdentifier() + " MODE " + ch->getName() + " :" + sign + m + "\r\n";
				broadcastToChan(ch, s, c, false);
			}
			else
			{
				s = ":42IRC 461 MODE :Not enough parameters\r\n"; send(c->getFd(), s.c_str(), s.size(), 0);
			}
			break ;

		case 'b':
			if (*pi < mp.size())
			{
				Client *cto = findClient(mp[*pi], 2);
				if (cto != NULL)
				{
					!sign.compare("+") ? ch->addBanned(cto->getFullIdentifier()) : ch->removeBan(cto->getFullIdentifier());
					s = ":" + c->getFullIdentifier() + " MODE " + ch->getName() + " :" + sign + m + " " + cto->getFullIdentifier() +"\r\n";
					broadcastToChan(ch, s, c, false);
					if (!sign.compare("+") && cto->isOnChannel(ch->getName()))
					{
						s.clear();
						s = ":" + c->getFullIdentifier() + " KICK " + ch->getName() + " " + cto->getNick() + " :Banned\r\n";
						broadcastToChan(ch, s, c, false);
						cto->removeChannel(ch->getName());
						ch->removeClient(cto->getFd());
						if (ch->isAnOperator(cto->getFd()))
							ch->removeOperator(cto->getFd());
					}
				}
				*pi += 1;
			}
			else
			{
				s = ":42IRC 461 MODE :Not enough parameters\r\n"; send(c->getFd(), s.c_str(), s.size(), 0);
			}
			break ;

		default:
		s = ":42IRC 501 " + c->getNick() + " :Unkown MODE flag\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		break;
	}
}

void	Server::operCmd(Message *mess, Client *c)
{
	std::string s;
	if (mess->params.size() < 2)
	{
		s = ":42IRC 461 OPER :Not enough parameters\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	std::string name = mess->params[0];
	std::string pass = mess->params[1];
	if (isServerOp(c->getFd()) == false)
	{
		if (!pass.compare(getPassword()))
		{
			s = ":42IRC 381 :You're now an IRC operator\r\n";
			s += ":" + c->getFullIdentifier() + " MODE " + c->getNick() + " :+o\r\n";
			send(c->getFd(), s.c_str(), s.size(), 0);
			addServerOp(c->getFd());
		}
		else
		{
			s = ":42IRC 464 " + c->getNick() + " :Password incorrect\r\n";
			send(c->getFd(), s.c_str(), s.size(), 0);
		}
	}
}

void	Server::listCmd(Message *mess, Client *c)
{
	std::ostringstream s;
	if (!mess->params.size())
	{
		for (int i = 0; i < _chV.size(); i++)
		{
			if (_chV[i]->isSecret() == false)
			{
				s << ":42IRC" << " 322 " + c->getNick() + " " << _chV[i]->getName() << " " << _chV[i]->getClients().size() << " :" << (_chV[i]->getTopic().size() > 0 ? _chV[i]->getTopic() + "\r\n" : "No topic is set\r\n");
				send(c->getFd(), s.str().c_str(), s.str().size(), 0);
				s.str("");
				s.clear();
			}
			else
			{
				s << ":42IRC 403 " + c->getNick()+ " " +_chV[i]->getName() + "\r\n";
				send(c->getFd(), s.str().c_str(), s.str().size(), 0);
				s.str("");
				s.clear();
			}
		}
		s << ":" + c->getFullIdentifier() + " 323 :End of /LIST\r\n";
		send(c->getFd(), s.str().c_str(), s.str().size(), 0);
	}
	else
	{
		std::string ut = mess->params[0];
		std::vector<std::string> v = ft_split((char*)ut.c_str(), ",");
		for (int i = 0; i < v.size(); i++)
		{
			if (channelExist(v[i]) == true)
			{
				Channel *ch = findChannel(v[i]);
				if (ch->isSecret() == false)
				{
					s << ":42IRC 322 " + c->getNick() + " " +  ch->getName() + " " << ch->getClients().size() << " :" + (ch->getTopic().size() > 0 ? ch->getTopic() + "\r\n" : "No topic is set\r\n");
					send(c->getFd(), s.str().c_str(), s.str().size(), 0);
					s.str("");
					s.clear();
				}
			}
			else
			{
				s << ":42IRC 403 " + c->getNick()+ " "  + v[i] + "\r\n";
				send(c->getFd(), s.str().c_str(), s.str().size(), 0);
				s.str("");
				s.clear();
			}
		}
		s << ":" + c->getFullIdentifier() + " 323 :End of /LIST\r\n";
		send(c->getFd(), s.str().c_str(), s.str().size(), 0);
	}
}

void	Server::namesCmd(Message *mess, Client *c)
{
	std::string ut = mess->params[0];
	std::vector<std::string> v1 = ft_split((char*)ut.c_str(), ",");

	std::string s;
	if (!v1.size())
	{
		s = ":42IRC 366 " + c->getNick() + " * :End of NAMES list\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
	}
	else
	{

		for (int i = 0; i < v1.size(); i++)
		{
			if (channelExist(v1[i]) == true)
			{
				Channel *ch = findChannel(v1[i]);
				if (ch->isMode('s') == false || c->isOnChannel(v1[i]) == true)
				{
					s = ":42IRC 353 " + c->getNick() + " " + (ch->isSecret() == false ? "= " : "@ ") + ch->getName() + " :";
					std::vector<int> v = ch->getClients();
					for (int i = 0; i < v.size(); i++)
					{
						Client *cl = findClient(v[i]);
						if (i + 1 < v.size())
							s+= (ch->isAnOperator(cl->getFd()) == true ? "@" : "") + (ch->canITalk(cl->getFd()) == true ? ("+" + cl->getNick()) : cl->getNick()) + " ";
						else
							s+= (ch->isAnOperator(cl->getFd()) == true ? "@" : "") + (ch->canITalk(cl->getFd()) == true ? ("+" + cl->getNick()) : cl->getNick()) + "\r\n";
					}
					s += ":42IRC 366 " + c->getNick() + " " + ch->getName() + " :End of NAMES list\r\n";
					send(c->getFd(), s.c_str(), s.size(), 0);
				}
			}
			else
			{
				s = ":42IRC 366 " + c->getNick() + " " + v1[i] + " :End of NAMES list\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
			}
		}
	}
}

void	Server::nickCmd(Message *mess, Client *c)
{
	std::string s;
	if (checkNick(mess->params[0], c->getFd()))
	{
		s = ":" + c->getFullIdentifier() + " NICK " + mess->params[0] + "\r\n"; 
		for (int i = 0; i < _cVec.size(); i++)
			send(_cVec[i]->getFd(), s.c_str(), s.size(), 0);
		c->setNick(mess->params[0]);
	}
}

void Server::topicCmd(Message *mess, Client *c)
{
	if (mess->params[0].size() == 0)
	{
		send(c->getFd(), "461 TOPIC :Not enough parameters\r\n", 35, 0);
		return ;
	}

	std::string channel = mess->params[0];
	std::string top = ReplyCreator(mess, c, 1);

	if (!top.compare("\r\n"))
		top.clear();

	std::string s;

	if (channelExist(channel) == true && c->isOnChannel(channel) == true)
	{
		Channel *ch = findChannel(channel);

		if (!top.size())
		{
			s = (ch->getTopic().size() > 0 ? (":42IRC 332 " + c->getNick() + " " + ch->getName() + " :" + ch->getTopic() + "\r\n") : (":42IRC 331 " + c->getNick() + " " + ch->getName() + " :No topic is set\r\n"));
			send(c->getFd(), s.c_str(), s.size(), 0);
			return ;
		}
		else
		{
			if (ch->isAnOperator(c->getFd()) == true || ch->isMode('t') == false || isServerOp(c->getFd()))
			{
				top.compare(":") == 0 ? ch->setTopic(NULL) : ch->setTopic(top.substr(1, std::string::npos));
				for (std::vector<int>::const_iterator it = ch->getClients().begin(); it != ch->getClients().end(); it++)
				{
					Client *cut = findClient(*it);
					s = (ch->getTopic().size() > 0 ? (":42IRC 332 " + cut->getNick() + " " + ch->getName() + " :" + ch->getTopic() + "\r\n") : (":42IRC 331 " + cut->getNick() + " " + ch->getName() + " :No topic is set\r\n")) + ":42IRC 333 " + c->getFullIdentifier() + " " + channel + " " + c->getNick() + " " + _displayTimestamp() + "\r\n";
					send(cut->getFd(), s.c_str(), s.size(), 0);
				}
			}
			else
			{
				s = ":42IRC 482 " + channel + " :You're not an operator\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
				return ;
			}

		}
	}
	else if (channelExist(channel) == false)
	{
		s = ":42IRC 403 " + c->getNick()+ " " + channel + "\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	else if (c->isOnChannel(channel) == false)
	{
		s = ":42IRC 442 " + channel + " :You're not on that channel\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
}

void	Server::partCmd(Message *mess, Client *c)
{
	std::string s;
	std::vector<std::string> ctp;

	if (mess->params.size() > 0)
	{
		std::string ut = mess->params[0];
		ctp = ft_split((char*)ut.c_str(), ",");
	}
	else
	{
		s = "461 PART :Not enough parameters\r\n";
		send(c->getFd(), s.c_str(), s.size(), 0);
		return ;
	}
	std::string reason = ReplyCreator(mess, c, 1);

	for (int i = 0; i < ctp.size(); i++)
	{
		if (channelExist(ctp[i]) == true)
		{

			if (c->isOnChannel(ctp[i]) == true)
			{
				s = ":" + c->getFullIdentifier() + " PART " + ctp[i] + (reason.size() > 0 ? " " + reason : "\r\n");
				Channel *ch = findChannel(ctp[i]);
				for (std::vector<int>::const_iterator it = ch->getClients().begin(); it != ch->getClients().end(); it++)
						send(*it, s.c_str(), s.size(), 0);
				c->removeChannel(ctp[i]);
				ch->removeClient(c->getFd());
				if (ch->isAnOperator(c->getFd()))
					ch->removeOperator(c->getFd());
				if (ch->getClients().size() == 0)
					deleteChannel(ch->getName());
			}
			else
			{
				s = ":42IRC 442 " + ctp[i] + " :You're not on that channel\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
			}
		}
		else
		{
			s = ":42IRC 403 " + c->getNick()+ " " + ctp[i] + "\r\n";
			send(c->getFd(), s.c_str(), s.size(), 0);
		}
	}

}

void	Server::noticeCmd(Message *mess, Client *c)
{
	std::string ut = mess->params[0];
	std::vector<std::string> target = ft_split((char*)ut.c_str(), ",");
	std::string msg = ReplyCreator(mess, c, 1);

	for (int i = 0; i < target.size(); i++)
	{
		if (target[i][0] != '#') //nick
		{
			std::string s;
			for (int j = 0; j < _cVec.size(); j++)
			{
				if (!_cVec[j]->getNick().compare(target[i]))
				{
					s = ":" + c->getFullIdentifier() + " NOTICE " + target[i] + " " + msg;
					send(_cVec[j]->getFd(), s.c_str(), s.size(), 0);
					break;
				}
			}
		}
		else //channel
		{
			std::string s;
			for (int j = 0; j < _chV.size(); j++)
			{
				if (!_chV[j]->getName().compare(target[i]) && ((c->isOnChannel(target[i]) == true || !_chV[i]->isMode('n')) && _chV[i]->isBanned(c->getFullIdentifier()) == false && (_chV[i]->canITalk(c->getFd()) == true || _chV[i]->isMode('m') == false)))
				{
					s = ":" + c->getFullIdentifier() + " NOTICE " + target[i] + " " + msg;
					for (std::vector<int>::const_iterator it = _chV[j]->getClients().begin(); it != _chV[j]->getClients().end(); it++)
					{
						if (*it != c->getFd())
							send(*it, s.c_str(), s.size(), 0);
					}
					break;
				}
			}
		}
	}
}

void	Server::privmsgCmd(Message *mess, Client *c)
{
	std::string ut = mess->params[0];
	std::vector<std::string> target = ft_split((char*)ut.c_str(), ",");
	std::string msg = ReplyCreator(mess, c, 1);

	if (msg.size() < 2)
		send(c->getFd(), ":42IRC 412 :No text to send\r\n", 30, 0);
	for (int i = 0; i < target.size(); i++)
	{
		if (target[i][0] != '#') //nick
		{
			int flag = 0;
			std::string s;
			for (int j = 0; j < _cVec.size(); j++)
			{
				if (!_cVec[j]->getNick().compare(target[i]))
				{
					s = ":" + c->getFullIdentifier() + " PRIVMSG " + target[i] + " " + msg;
					send(_cVec[j]->getFd(), s.c_str(), s.size(), 0);
					flag = 1;
					break;
				}
			}
			if (flag == 0)
			{
				s = ":42IRC 401 "+ c->getNick() + " " + target[i] + " :No such nick/channel\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
			}
		}
		else //channel
		{
			int flag = 0;
			std::string s;
			for (int j = 0; j < _chV.size(); j++)
			{
				if (!_chV[j]->getName().compare(target[i]) && ((c->isOnChannel(target[i]) == false && _chV[i]->isMode('n') == true) || _chV[i]->isBanned(c->getFullIdentifier()) == true || (_chV[i]->canITalk(c->getFd()) == false && _chV[i]->isMode('m') == true)))
				{
					flag = 1;
					s = ":42IRC 404 " + target[i] + "\r\n";
					send(c->getFd(), s.c_str(), s.size(), 0);
					break;
				}
				else if (!_chV[j]->getName().compare(target[i]) && ((c->isOnChannel(target[i]) == true || !_chV[i]->isMode('n')) && _chV[i]->isBanned(c->getFullIdentifier()) == false && (_chV[i]->canITalk(c->getFd()) == true || _chV[i]->isMode('m') == false)))
				{
					s = ":" + c->getFullIdentifier() + " PRIVMSG " + target[i] + " " + msg;
					for (std::vector<int>::const_iterator it = _chV[j]->getClients().begin(); it != _chV[j]->getClients().end(); it++)
					{
						if (*it != c->getFd())
							send(*it, s.c_str(), s.size(), 0);
					}
					flag = 1;
					break;
				}
			}
			if (flag == 0)
			{
				s = ":42IRC 401 "+ c->getNick() + " " + target[i] + " :No such nick/channel\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
			}
		}
	}
}

void initializePartAll(Message *alt, Client *c)
{
	alt->command = "PART";
	std::string s;
	for (std::vector<std::string>::const_iterator it = c->getClientChannel().begin(); it != c->getClientChannel().end(); it++)
	{
		if (it + 1 != c->getClientChannel().end())
			s+= *it + ",";
		else
			s+= *it;
	}
	alt->params.push_back(s);
}

void	Server::joinCmd(Message *mess, Client *c)
{
	if (!mess->params[0].compare("#0"))
	{
		Message alt;
		initializePartAll(&alt, c);
		partCmd(&alt, c);
		return ;
	}

	if (!mess->params.size())
	{
		send(c->getFd(), "461 JOIN :Not enough parameters\r\n", 34, 0);
		return ;
	}
	std::vector<std::string>	channels;
	std::vector<std::string>	keys;

	char ut[4096];
	bzero(ut, 4096);
	memcpy(ut, mess->params[0].c_str(), mess->params[0].size());
	channels = ft_split(ut, ",");

	bzero(ut, 4096);
	memcpy(ut, mess->params[1].c_str(), mess->params[1].size());
	if (ut[0] != 0)
		keys = ft_split(ut, ",");

	for (int i = 0; i < channels.size(); i++)
	{
		if (channelExist(channels[i]) == true)
		{
			Channel *ch = findChannel(channels[i]);
			if ((ch->getKey().size() == 0 || (i < keys.size() && keys[i].compare(ch->getKey()) == 0) || ch->isMode('k') == false) &&  ch->isBanned(c->getFullIdentifier()) == false && c->isOnChannel(ch->getName()) == false)
			{
				std::string s = ":" + c->getFullIdentifier() + " JOIN " + channels[i] + "\r\n";
				send (c->getFd(), s.c_str(), s.size(), 0);
				std::vector<int> clients = ch->getClients();
				for (int i = 0; i < clients.size(); i++)
					send(clients[i], s.c_str(), s.size(), 0);
				ch->setNewClient(c->getFd());
				c->setNewClientChannel(channels[i]);
				sendChannelInformation(c, ch);
			}
			else if (ch->isBanned(c->getFullIdentifier()) == true && c->isOnChannel(ch->getName()) == false)
			{
				std::string s = ":42IRC 474 " + c->getNick() + " " + channels[i] + " :Cannot join channel (+b)\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
			}
			else if (ch->isMode('k') == true && (ch->getKey().size() != 0 || i > keys.size() || keys[i].compare(ch->getKey()) != 0) && c->isOnChannel(ch->getName()) == false)
			{
				std::string s = ":42IRC 475 " + c->getNick() + " " + channels[i] + " :Cannot join channel (+k)\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
			}
		}
		else
		{
			Channel *chan;

			if (i < keys.size())
				chan = new Channel(channels[i], keys[i]);
			else
				chan = new Channel(channels[i]);
			
			_chV.push_back(chan);
			std::string s = ":" + c->getFullIdentifier() + " JOIN " + channels[i] + "\r\n";
			send (c->getFd(), s.c_str(), s.size(), 0);
			chan->setNewClient(c->getFd());
			c->setNewClientChannel(channels[i]);
			chan->setNewOperator(c->getFd());
			sendChannelInformation(c, chan);
			chan->setModes(i < keys.size() ? "+otnk" : "+otn");
		}
	}
	return ;

}

void	Server::sendChannelInformation(Client *c, Channel *ch)
{
	std::string s;

	s = (ch->getTopic().size() > 0 ? (":42IRC 332 " + c->getNick() + " " + ch->getName() + " :" + ch->getTopic() + "\r\n") : (":42IRC 331 " + c->getNick() + " " + ch->getName() + " :No topic is set\r\n"));
	send(c->getFd(), s.c_str(), s.size(), 0);

	s.clear();
	s = ":42IRC 353 " + c->getNick() + " " + (ch->isSecret() == false ? "= " : "@ ") + ch->getName() + " :";
	std::vector<int> v = ch->getClients();
	for (int i = 0; i < v.size(); i++)
	{
		Client *cl = findClient(v[i]);
		if (i + 1 < v.size())
			s+= (ch->isAnOperator(cl->getFd()) == true ? "@" : "") + (ch->canITalk(cl->getFd()) == true ? ("+" + cl->getNick()) : cl->getNick()) + " ";
		else
			s+= (ch->isAnOperator(cl->getFd()) == true ? "@" : "") + (ch->canITalk(cl->getFd()) == true ? ("+" + cl->getNick()) : cl->getNick()) + "\r\n";
	}
	send(c->getFd(), s.c_str(), s.size(), 0);

	s.clear();
	s = ":42IRC 366 " + c->getNick() + " " + ch->getName() + " :End of NAMES list\r\n";
	send(c->getFd(), s.c_str(), s.size(), 0);
}

void	Server::quitCmd(Message *mess, Client *c, fd_set *currentsockets)
{
	std::string ss;
	ss = ":" +c->getFullIdentifier() + " QUIT ";
	for (int i = 0; i < mess->params.size(); i++)
	{
		if (i + 1 < mess->params.size())
			ss += mess->params[i] + " ";
		else
			ss += mess->params[i] + "\r\n";
	}
	send(c->getFd(),ss.c_str() , ss.size(), 0);
	for (std::vector<std::string>::const_iterator it = c->getClientChannel().begin(); it != c->getClientChannel().end(); it++)
	{
		Channel *ch = findChannel(*it);
		for (std::vector<int>::const_iterator i = ch->getClients().begin(); i != ch->getClients().end(); i++)
		{
			if (*i != c->getFd())
				send(*i, ss.c_str(), ss.size(), 0);
		}
		ch->removeClient(c->getFd());
		if (ch->isAnOperator(c->getFd()) == true)
			ch->removeOperator(c->getFd());
		if (!ch->getClients().size())
			deleteChannel(ch->getName());
	}
	closeClientConnection(c->getFd(), currentsockets);
}
