#ifndef SERVER_HPP
# define SERVER_HPP


#include "Client.hpp"
#include "main.hpp"
#include <vector>


class Client;

class Server
{
	public :
		Server() {};
		~Server() {};

		// Getter and setter 
		void	setPort(const int & pn)
		{
			port = pn;
		};

		const int & getPort(void) const { return this->port; };

		void	setPassword(const char *pass)
		{
			Password = std::string(pass);
		}

		void	addClient(int fd, std::string Host)
		{
			Client *c = new Client(fd, Host);
			_cVec.push_back(c);
		}

		bool getClientRegistrationStatus(int fd) const { 
			for (std::vector<Client*>::const_iterator _cIt = _cVec.begin(); _cIt != _cVec.end(); _cIt++)
			{
				Client *c = *_cIt;
				if (c->getFd() == fd)
					return c->getIsRegistered();
			}
			return false;
		}

		const std::string & getPassword(void) const { return this->Password; };


		// Anything else in the Server.cpp file

		void	launch();
		const Client & findClient(int fd) const;
		void	closeClientConnection(int fd);


	private :
		int	port;
		std::string Password;
		std::vector<Client*> _cVec;
		
};

#endif