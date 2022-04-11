#include "Server.hpp"

void    Server::launch()
{
    //Create the socket
	int	listeningSock = socket(AF_INET, SOCK_STREAM, 0);
	int justforut= 1;
	setsockopt(listeningSock, SOL_SOCKET, SO_REUSEADDR, &justforut, sizeof(int));
	if (listeningSock == -1)
	{
		std::cout << "Can't create a socket. Quitting." << std::endl;
		exit(1);
	}
	//Bind an ip and a port to the socket
	sockaddr_in serv_addr, client_addr;
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(getPort());
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(listeningSock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cout << "Error bindind the socket" << std::endl;
		exit(1);
	}

	//Tell that the socket is for listening
	listen(listeningSock, SOMAXCONN);
	fcntl(listeningSock, F_SETFL, O_NONBLOCK);

	int client_len = sizeof(client_addr); 


	fd_set currentSockets, readySockets;

	FD_ZERO(&currentSockets);
	FD_SET(listeningSock, &currentSockets);	

	int maxfds = listeningSock, socket_connection_fd;

	while (1)
	{
		readySockets = currentSockets;

		if (select(maxfds + 1, &readySockets, NULL, NULL, NULL) < 0)
		{
			perror("select error");
			exit(1);
		}

		for (int i = 0; i <= maxfds; i++)
		{
			if (FD_ISSET(i, &readySockets))
			{
				if (i == listeningSock)
				{
					socket_connection_fd = accept(listeningSock, (struct sockaddr*)&client_addr, (socklen_t *)&client_len);
					fcntl(socket_connection_fd, F_SETFL, O_NONBLOCK);
					if (socket_connection_fd == -1)
						perror("Accept socket error");
					
					char host[4096];
					getnameinfo((sockaddr *)&client_addr, client_len, host, 4096, NULL, 0, NI_NAMEREQD);
					addClient(socket_connection_fd, std::string(host));

					maxfds = maxfds > socket_connection_fd ? maxfds : socket_connection_fd;

					FD_SET(socket_connection_fd, &currentSockets);

					std::cout << "New connection accepted" << std::endl;
				}
				else
				{
					char buf[4096];
					bzero(buf, 4096);
					int bytes_read = recv (i, buf, 4096, 0);
					
					if (bytes_read <= 0)
					{
						closeClientConnection(i, &currentSockets);
						FD_CLR(i, &currentSockets);
					}
					else
					{
						Client *c = findClient(i);
						std::vector<std::string> v;
						c->addPersonalBuff(std::string(buf));

						if (c->messageReady() == true)
						{
							std::cout<< c->getPersonalBuff() << std::endl;
							if (c->getIsRegistered() == false)
							{
								v = ft_split((char *)c->getPersonalBuff().c_str(), " :\r\n");
								login(c, i, v);
							}
							else if (c->getIsRegistered() == true)
							{
									Message mess;
									v = ft_split((char *)c->getPersonalBuff().c_str(), " \r\n");
									if (v.size() > 0)
									{
										initializeMess(&mess, v);
										MessageHandler(&mess, c, &currentSockets);
									}
							}
							v.clear();
							c->clearPersonalBuff();
						}
					}
				}
			}
		}
	}
}

void	Server::MessageHandler(Message *mess, Client *c, fd_set *currentsockets)
{
	
	if (!mess->command.compare("PING"))
		Replyer(PING, c, mess, currentsockets);
	else if (!mess->command.compare("JOIN"))
		Replyer(JOIN, c, mess, currentsockets);
	else if(!mess->command.compare("QUIT"))
		Replyer(QUIT, c, mess, currentsockets);
	else if(!mess->command.compare("PRIVMSG"))
		Replyer(PRIVMSG, c, mess, currentsockets);
	else if(!mess->command.compare("PART"))
		Replyer(PART, c, mess, currentsockets);
	else if(!mess->command.compare("TOPIC"))
		Replyer(TOPIC, c, mess, currentsockets);
	else if(!mess->command.compare("NICK"))
		Replyer(NICK, c, mess, currentsockets);
	else if(!mess->command.compare("NAMES") || !mess->command.compare("names"))
		Replyer(NAMES, c, mess, currentsockets);
}

void	Server::Replyer(int cmd, Client *c, Message *mess, fd_set *currentsockets)
{
	switch(cmd)
	{
		case PING : send(c->getFd(),"PONG\r\n" , 7, 0); break;
		
		case JOIN : joinCmd(mess, c); break;

		case PRIVMSG : privmsgCmd(mess, c); break;
		
		case PART : partCmd(mess, c); break;

		case QUIT : quitCmd(mess, c, currentsockets); break ;

		case TOPIC : topicCmd(mess, c); break;

		case NICK : nickCmd(mess, c); break;

		case NAMES : namesCmd(mess, c); break;

		default : break;
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

Channel * Server::findChannel(std::string name) const
{
	for (int i = 0; i < _chV.size(); i++)
	{
		if (_chV[i]->getName().compare(name) == 0)
			return _chV[i];
	}
	return NULL;
} 

std::string Server::ReplyCreator(Message *mess, Client *c, int i)
{
	std::string ss;
	
	if (i == 0)
		ss += mess->command + " ";
	for (std::vector<std::string>::iterator it = mess->params.begin() + i; it != mess->params.end(); it++)
	{
		if (it + 1 != mess->params.end())
			ss += *it + " ";
		else
			ss += *it;
	}
	ss += "\r\n";
	return ss;
}

void	Server::login(Client *c, int event_fd, std::vector<std::string> v)
{
	int kok = 0;
	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
	{
		if (!it->compare("PASS"))
		{
			kok = 1;
			it++;
			if (getPassword().compare(*it) != 0)
			{
				send(event_fd, ":42IRC 464 : Password incorrect\r\n", 28, 0);
				close(event_fd);
				_cVec.pop_back();
				break ;
			}
			else
				c->setPassed(true);
		}
		else if (!it->compare("NICK"))
		{
			it++;
			if (checkNick(*it, event_fd))
				c->setNick(*it);
		}
		else if (!it->compare("USER"))
		{
			it++;
			if (checkUser(*it, event_fd) == 1)
				c->setUsername(*it);
			else if (checkUser(*it, event_fd) == 2)
			{
				std::string str = *it;
				str.resize(12);
				c->setUsername(str);
			}	
		}
	}
	if (kok == 0 && c->getPassed() == false)
		send(c->getFd(), "461 PASS :Not enough parameters\r\n", 34, 0);
	if (c->getNick().size() && c->getUsername().size() && c->getPassed() == true)
	{
		c->setFullIdentifier();
		std::ostringstream RPL;
		RPL << ":42IRC 001 " << c->getNick() << " :Welcome to the 42IRC Network, " << c->getNick() << "!" << c->getUsername() << "@" << c->getHostAddress() <<"\r\n"
		<<":42IRC 002 " << c->getNick() << " :Your host is 42IRC, running version 1.2\r\n"
		<<":42IRC 003 " << c->getNick() << " :This server was created " << getCreationTime() << "\r\n"
		<<":42IRC 004 " << c->getNick() << " 42IRC 1.2 o boktmvnls ovkl\r\n";
		send(event_fd, RPL.str().c_str(), RPL.str().size(), 0);
		c->setIsRegistered(true);
	}
}

int		Server::checkUser(std::string user, int fd)
{
	if (user.size() <= 0)
	{
		send(fd, "461 USER : Not enough parameters\r\n", 35, 0);
		return 0;
	}
	else if (user.size() > 12)
	{
		return 2;
	}
	return 1;
}

int		Server::checkNick(std::string nick, int fd)
{
	if (nick.size() <= 0)
	{
		send(fd, "431 : No nickname given\r\n", 26, 0);
		return 0;
	}
	if (nick.size() > 9)
	{send(fd, "432 : Erroneous nickname\r\n", 27, 0);
			return 0;}
	for (int i = 0; i < nick.size(); i++)
	{
		if (!isprint(nick[i]) || !nick.compare(i, i+1, " "))
		{
			send(fd, "432 : Erroneous nickname\r\n", 27, 0);
			return 0;
		}
	}
	for (std::vector<Client*>::iterator it = _cVec.begin(); it != _cVec.end(); it++)
	{
		Client *c = *it;
		if (c->getFd() != fd && !c->getNick().compare(nick))
		{
			send(fd, "433 : Nickname already in use\r\n", 32, 0);
			return 0;
		}
	}
	return 1;
}

void	Server::closeClientConnection(int fd, fd_set *currentsocket)
{
	std::cout << "Client has disconnected" <<std::endl;
	int i = 0;
	for (std::vector<Client*>::iterator it = _cVec.begin(); it != _cVec.end(); it++)
	{
		if (_cVec[i]->getFd() == fd)
		{
			delete *it;
			_cVec.erase(it);
			break;
		}
		i++;
	}
	close(fd);
	FD_CLR(fd, currentsocket);
}

Client * Server::findClient(int fd) const
{
	Client *c;

	for (int i = 0; i < _cVec.size(); i++)
	{
		if (_cVec[i]->getFd() == fd)
			return _cVec[i];
	}
	return NULL;
}
