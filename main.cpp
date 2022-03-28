#include "main.hpp"

int	isNum(const char *s)
{
	for (int i = 0; s[i]; i++)
	{
		if (isdigit(s[i]) == 0)
			return 0;
	}

	return 1;
}

int firstcheck(Server *serv, char **argv)
{
	int	pn;

	if (isNum(argv[1]) && (pn = atoi(argv[1])) > 1023 && pn < 65535)
	{
		serv->setPort(pn);
	}
	else
	{
		std::cout << "Wrong port. Quitting." << std::endl;
		return 1;
	}

	if (strlen(argv[2]) > 0)
		serv->setPassword(argv[2]);
	else
	{
		std::cout << "Password need to be setted. Quitting." << std::endl;
		return 1;
	}

	return 0;
}

int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Program need to be runned as follows : ./ircserv <port> <password>" << std::endl;
		exit(1);
	}

	Server	serv;
	int clients = 0;
	int	fd_array[SOMAXCONN + 1];
	memset(fd_array, -1, SOMAXCONN);

	if (firstcheck(&serv, argv) == 1)
		exit(1);
	//Create the socket
	int	listeningSock = socket(AF_INET, SOCK_STREAM, 0);
	int justforut= 1;
	setsockopt(listeningSock, SOL_SOCKET, SO_REUSEADDR, &justforut, sizeof(int));
	if (listeningSock == -1)
	{
		std::cout << "Can't create a socket. Quitting." << std::endl;
		exit(1);
	}
	fd_array[clients++] = listeningSock;
	//Bind an ip and a port to the socket
	sockaddr_in serv_addr, client_addr;
	bzero((char *)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(serv.getPort());
	serv_addr.sin_addr.s_addr = INADDR_ANY; //need to use inet_addr or inet_aton but idk how to have the ip address of the machine :/

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
				fd_array[clients++] = socket_connection_fd;
				
				// char host[4096];
				// getnameinfo((sockaddr *)&client_addr, client_len, host, 4096, NULL, 0, NI_NAMEREQD);

				//std::cout << host << std::endl;

				std::cout << "New connection accepted" << std::endl;

				EV_SET(change_event, socket_connection_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
				if (kevent(kq, change_event, 1, NULL, 0, NULL) < 0)
					perror("kevent error");
				
				std::string welcomeMsg = "Welcome to the server";
				send(socket_connection_fd, welcomeMsg.c_str(), welcomeMsg.size(), 0);

				
			}

			else if (change_event[i].filter & EVFILT_READ)
			{
					char buf[4096];
					bzero(buf, 4096);
					recv(event_fd, buf, sizeof(buf), 0);
					
					std::cout << buf << std::endl;

					for (int i = 1; i < clients; i++)
					{
						int outSock = fd_array[i];
						std::ostringstream ss;
						ss << "USER" << event_fd << " : " << buf;
						std::string msg = ss.str();
						if (outSock != event_fd)
							send(outSock, msg.c_str(), msg.size(), 0);
					}
				
			}
		}
	}

	return 0;
}