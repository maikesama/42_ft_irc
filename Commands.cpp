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
			if (channelExist(v1[i]) == true /*&& c->isOnChannel(v[i]) == true*/) //+ Channel is not secret
			{
				Channel *ch = findChannel(v1[i]);
				s = ":42IRC 353 " + c->getNick() + " " + (ch->isSecret() == false ? "= " : "@ ") + ch->getName() + " :";
				std::vector<int> v = ch->getClients();
				for (int i = 0; i < v.size(); i++)
				{
					Client *cl = findClient(v[i]);
					if (i + 1 < v.size())
						s+= (ch->isAnOperator(cl->getFd()) == true ? "@" + cl->getNick() : cl->getNick()) + " ";
					else
						s+= (ch->isAnOperator(cl->getFd()) == true ? "@" + cl->getNick() : cl->getNick()) + "\r\n";
				}
				s += ":42IRC 366 " + c->getNick() + " " + ch->getName() + " :End of NAMES list\r\n";
				send(c->getFd(), s.c_str(), s.size(), 0);
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
			if (ch->isAnOperator(c->getFd()) == true || ch->isMode('t') == false)
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
				s = ":42IRC 442 " + channel + " :You're not an operator\r\n";
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
				if (!_chV[j]->getName().compare(target[i]) && c->isOnChannel(target[i]) == false)
				{
					flag = 1;
					s = ":42IRC 404 " + target[i] + " :Cannot send to channel\r\n";
					send(c->getFd(), s.c_str(), s.size(), 0);
					break;
				}
				else if (!_chV[j]->getName().compare(target[i]) && c->isOnChannel(target[i]) == true)
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
			if (ch->getKey().size() == 0 || (i < keys.size() && keys[i].compare(ch->getKey()) == 0))
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
			else
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
			s.clear();
			s = ":42IRC 324 " + c->getNick() + " " + chan->getName() + (i < keys.size() ? (" otnk " + chan->getKey() + "\r\n") : " otn\r\n");
			chan->setModes(i < keys.size() ? "+otnk" : "+otn");
			send(c->getFd(), s.c_str(), s.size(), 0);
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
			s+= (ch->isAnOperator(cl->getFd()) == true ? "@" + cl->getNick() : cl->getNick()) + " ";
		else
			s+= (ch->isAnOperator(cl->getFd()) == true ? "@" + cl->getNick() : cl->getNick()) + "\r\n";
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
