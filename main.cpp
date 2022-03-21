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

int firstcheck(Server serv, char **argv)
{
	int	pn;

	if (isNum(argv[1]) && (pn = atoi(argv[1])) > 0 && pn < 65535)
		serv.setPort(pn);
	else
	{
		std::cout << "Wrong port. Quitting." << std::endl;
		return 1;
	}

	if (strlen(argv[2]) > 0)
		serv.setPassword(argv[2]);
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
		return 1;
	}

	Server	serv;

	if (firstcheck(serv, argv) == 1)
		return 1;

	//Create the socket
	int	listeningSock = socket(AF_INET, SOCK_STREAM, 0);
	if (listeningSock == -1)
	{
		std::cout << "Can't create a socket. Quitting." << std::endl;
		return 1;
	}

	//Bind an ip and a port to the socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(serv.getPort());
	hint.sin_addr.s_addr = INADDR_ANY; //need to use inet_addr or inet_aton but idk how to have the ip address of the machine :/

	bind(listeningSock, (sockaddr*)&hint, sizeof(hint));

	//Tell that the socket is for listening
	listen(listeningSock, SOMAXCONN);

	fd_set master;

	FD_ZERO(&master);

	FD_SET(listeningSock, &master);

	while (1)
	{
		fd_set copy = master;

		int socketCount = select(0, &copy, NULL, NULL, NULL);

		for (int i = 0; i < socketCount; i++)
		{
			int sock = copy.fd_array[i];
			if (sock == listeningSock)
			{
				//Accept and add the new connection to the list of connected clients
				int	client = accept(listeningSock, NULL, NULL);
				FD_SET(client, &master);
				std::string WelcomeMsg = "You're now connected and ready to talk!";
				send(client, WelcomeMsg.c_str(), WelcomeMsg.size() + 1, 0);
			}
			else
			{
				char	buf[4096];

				memset(buf, 0, 4096);
				//Accept and send new message to the other clients
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					close(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					for (int i = 0; i < socketCount; i++)
					{
						int outSock = master.fd_array[i];
						if (outSock != listeningSock && outSock != sock)
						{
							send(outSock, buf, 4096, 0);
						}

					}
				}
			}
		}
	}

}