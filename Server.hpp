#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <cstdlib>

class Server
{
	public :
		Server() {};
		~Server() {};

		void	setPort(const int & pn)
		{
			port = pn;
		};

		const int & getPort(void) const { return this->port; };

		void	setPassword(const char *pass)
		{
			Password = std::string(pass);
		}

		const std::string & getPassword(void) const { return this->Password; };

	private :
		int	port;
		std::string Password;

};

#endif