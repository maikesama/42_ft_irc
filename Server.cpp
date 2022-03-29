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

		new_events = kevent(kq, NULL, 0, event, 1, NULL);
	
		if ( new_events == -1)
		{
			perror("kevents");
			exit(1);
		}

		for(int i = 0; new_events > i; i++)
		{
			int event_fd = event[i].ident;

			if (event[i].flags & EV_EOF)
			{
				std::cout << "Client has disconnected" <<std::endl;
				close(event_fd);
			}

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
					if (getClientRegistrationStatus(event_fd) == false)
					{
						send(event_fd, "451 : not registered\r\n", 24, 0);
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
                                _cVec.back().setNick(std::string(tok));
							}
                            else if (!tmp.compare("USER"))
                            {
                                tok = strtok(NULL, " :\r\n");
                                _cVec.back().setUsername(std::string(tok));
                            }
                            else if (!tmp.compare(0, 3, "1,8"))
                            {
                                _cVec.back().setRealName(std::string(tok).substr(3, std::string(tok).size() - 3));
                            }
							tok = strtok(NULL, " :\r\n");
						}
                        // do it bettefr with nick eccc..
                        send(event_fd, "001 : Welcome\r\n", 17, 0);
                        _cVec.back().setIsRegistered(true);
					}
			}
		}
	}
}