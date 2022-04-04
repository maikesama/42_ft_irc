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
					std::cout << buf << std::endl;
					
					if (bytes_read <= 0)
					{
						closeClientConnection(i, &currentSockets);
						FD_CLR(i, &currentSockets);
					}
					else
					{
						Client *c(&findClient(i));

						std::vector<std::string> v = ft_split(buf, " :\r\n");

						if (c->getIsRegistered() == false)
							login(c, i, v);
						else if (c->getIsRegistered() == true)
							MessageHandler(v, c, i, &currentSockets);
						v.clear();
					}
				}
			}
		}
	}
}

void	Server::MessageHandler(std::vector<std::string> v, Client *c, int fd, fd_set *currentsockets)
{
	
	if (!v[0].compare("PING"))
		Replyer(PING, c, v, currentsockets);
	else if (!v[0].compare("JOIN"))
		Replyer(JOIN, c, v, currentsockets);
	else if(!v[0].compare("QUIT"))
		Replyer(QUIT, c, v, currentsockets);
}

void	Server::Replyer(int cmd, Client *c, std::vector<std::string> v, fd_set *currentsockets)
{
	std::string ss;

	switch(cmd)
	{
		case PING :
			ss = "PONG 42IRC\r\n"; 
			send(c->getFd(),ss.c_str() , ss.size(), 0);
			break;
		
		case JOIN :

			ss = ":" + c->getFullIdentifier() + " " + ReplyCreator(v, c, 0);
			//ss.append(ReplyCreator(v, c, 0));
			//ss.append("332 <client> <channel> :<topic> \r\n 353 <client> <symbol> <channel> :[prefix]<nick>{ [prefix]<nick>}\r\n 366 <client> <channel> :End of /NAMES list\r\n ");
			// ss.append(" #pol : topic is not setted yet\r\n ");
			send(c->getFd(), ss.c_str(), ss.size(), 0);
			std::cout << ss <<ss.size()<< std::endl;
			break;

		case QUIT :
			ss = "ERROR Closing link : " + c->getFullIdentifier() + " " + ReplyCreator(v, c, 1);
			send(c->getFd(),ss.c_str() , ss.size(), 0);
			closeClientConnection(c->getFd(), currentsockets);
			break ;

		default : break;
	}
}

std::string Server::ReplyCreator(std::vector<std::string> v, Client *c, int i)
{
	std::string ss;
	
	for (std::vector<std::string>::iterator it = v.begin() + i; it != v.end(); it++)
	{
		if (it + 1 != v.end())
			ss += *it + " ";
		else
			ss += *it;
	}
	ss += "\r\n";
	return ss;
}

void	Server::login(Client *c, int event_fd, std::vector<std::string> v)
{
	// if (c->getFirst() == false)
	// {
	// 	send(event_fd, "451 : not registered\r\n ", 24, 0);
	// 	c->setFirst();
	// }
	
	for (std::vector<std::string>::iterator it = v.begin(); it != v.end(); it++)
	{
		if (!it->compare("PASS"))
		{
			it++;
			if (getPassword().compare(*it) != 0)
			{
				send(event_fd, "464 : Password incorrect\r\n ", 28, 0);
				close(event_fd);
				_cVec.pop_back();
				break ;
			}
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
		//set RealName not explicitly required
	}
	if (c->getNick().size() && c->getUsername().size())
	{
		c->setFullIdentifier();
		std::ostringstream RPL;
		RPL << "001 : Welcome to the 42IRC Network, " << c->getNick() <<"!"<<c->getUsername()<<"@"<<c->getHostAddress()<<"\r\n"
		<< "002 : Your host is 42IRC, running version 1.2\r\n003 : This server was created " << getCreationTime() << "\r\n"
		<< "004 " << c->getFullIdentifier() << " 42IRC 1.2 o boktmvnls\r\n";
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
	for (int i = 0; i < nick.size(); i++)
	{
		if (!isprint(nick[i]))
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

Client & Server::findClient(int fd) const
{
	Client *c;

	for (std::vector<Client*>::const_iterator _cIt = _cVec.begin(); _cIt != _cVec.end(); _cIt++)
	{
		c = *_cIt;
		if (c->getFd() == fd)
			return *c;
	}
	return *c;
}
