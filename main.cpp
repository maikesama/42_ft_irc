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

const std::vector<std::string> ft_split(char *str, const char *ctos)
{
	std::vector<std::string> v;
	char *tok;
	tok = strtok(str, ctos);
	v.push_back(std::string(tok));
	while (tok != NULL)
	{
		tok = strtok(NULL, ctos);
		if (tok == NULL)
			break;
		v.push_back(std::string(tok));
	}
	return (v);
}

int	main(int argc, char **argv)
{
	if (argc != 3)
	{
		std::cout << "Program need to be runned as follows : ./ircserv <port> <password>" << std::endl;
		exit(1);
	}

	Server	serv;

	if (firstcheck(&serv, argv) == 1)
		exit(1);
	serv.setCreationTime();
	serv.launch();

}