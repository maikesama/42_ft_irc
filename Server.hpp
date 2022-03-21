#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <cstdlib>

class Server
{
	public :
		Server() {};
		~Server() {};

		void	setPort(int pn)
		{
			port = pn;
		};

		const int & getPort(void) const { return port; };

		void	setPassword(const char *pass)
		{
			Password = std::string(pass);
		}

		const std::string & getPassword(void) const { return Password; };

	private :
		int	port;
		std::string Password;


};

#endif