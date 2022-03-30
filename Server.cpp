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


	int kq = kqueue();
	struct kevent change_event[SOMAXCONN],
        		event[SOMAXCONN];

	EV_SET(change_event, listeningSock, EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, 0);

	if (kevent(kq, change_event, 1, NULL, 0, NULL) == -1)
	{
		perror("kevent");
		exit(1);
	}		

	int new_events = 0, socket_connection_fd;

	while (1)
	{

		if ((new_events = kevent(kq, NULL, 0, event, 1, NULL)) == -1)
		{
			perror("kevents");
			exit(1);
		}

		for(int i = 0; new_events > i; i++)
		{
			int event_fd = event[i].ident;

			if (event[i].flags & EV_EOF)
				closeClientConnection(event_fd);
				
			else if (event_fd == listeningSock)
			{
				socket_connection_fd = accept(listeningSock, (struct sockaddr*)&client_addr, (socklen_t *)&client_len);
				fcntl(socket_connection_fd, F_SETFL, O_NONBLOCK);
				if (socket_connection_fd == -1)
					perror("Accept socket error");
				
				char host[4096];
				getnameinfo((sockaddr *)&client_addr, client_len, host, 4096, NULL, 0, NI_NAMEREQD);
				addClient(socket_connection_fd, std::string(host));

				std::cout << "New connection accepted" << std::endl;

				EV_SET(change_event, socket_connection_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
				if (kevent(kq, change_event, 1, NULL, 0, NULL) < 0)
					perror("kevent error");
			}

			else if (change_event[i].filter & EVFILT_READ)
			{
					char buf[4096];
					bzero(buf, 4096);
					recv (event_fd, buf, 4096, 0);
					std::cout << buf << std::endl;
					Client c(findClient(event_fd));

					if (c.getIsRegistered() == false)
					{
						if (c.getFirst() == false)
						{
							send(event_fd, "451 : not registered\r\n", 24, 0);
							c.setFirst();
						}
						char *tok;
						tok = strtok(buf, " :\r\n");
						while (tok != NULL)
						{
							std::string tmp = std::string(tok);

							if (!tmp.compare("PASS"))
							{
								tok = strtok(NULL, " :\r\n");
								if (getPassword().compare(std::string(tok)) != 0)
								{
									send(event_fd, "464 : Password incorrect\r\n", 28, 0);
									close(event_fd);
                                    _cVec.pop_back();
									break ;
								}
							}
                            // Check parameters
							else if (!tmp.compare("NICK"))
							{
								tok = strtok(NULL, " :\r\n");
                                c.setNick(std::string(tok));
							}
                            else if (!tmp.compare("USER"))
                            {
                                tok = strtok(NULL, " :\r\n");
                                c.setUsername(std::string(tok));
                            }
                            else if (!tmp.compare(0, 3, "1,8")) //its not 1,8 in every case
                            {
                                c.setRealName(std::string(tok).substr(3, std::string(tok).size() - 3));
                            }
							tok = strtok(NULL, " :\r\n");
						}
                        // do it bettefr with nick eccc..
						if (c.getNick().size() && c.getUsername().size())
						{
							std::ostringstream RPL;
							RPL << "001 : Welcome to the 42IRC Network, " << c.getNick() <<"[!"<<c.getUsername()<<"@"<<c.getHostAddress()<<"]\r\n";
							send(event_fd, RPL.str().c_str(), RPL.str().size(), 0);
							// send(event_fd, "002 : your host is 42IRC\r\n", 27, 0);
							// std::cout << c.getNick() << std::endl;
							// send(event_fd, "003 : this server was created <timestamp>\r\n", 46, 0);
							// send(event_fd, "004 : server info\r\n", 22, 0);
							// send(event_fd, "005 : tokens are supported\r\n", 33, 0);
							send(event_fd, "002 : your host is 42IRC\r\n 003 : this server was created <timestamp>\r\n 004 : server info\r\n 005 : tokens are supported\r\n", 120, 0);
							c.setIsRegistered(true);
						}
					}
			}
		}
	}
}

void	Server::closeClientConnection(int fd)
{
	std::cout << "Client has disconnected" <<std::endl;
	close(fd);
}

const Client & Server::findClient(int fd) const
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
