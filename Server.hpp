#ifndef SERVER_HPP
# define SERVER_HPP


#include "Client.hpp"
#include "Channel.hpp"
#include "Commands.hpp"
#include "main.hpp"
#include <vector>
#include <ctime>


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

		void	setCreationTime()
		{
			const std::time_t   t = std::time(nullptr);
			char            str[80];
			bzero(str, 80);

			if (std::strftime(str, 80, "%c", std::localtime(&t)))
				CreationTime = std::string(str);
		}

		const std::string & getCreationTime() const { return CreationTime; }

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

		const std::string    _displayTimestamp( void )
		{
			const std::time_t   t = std::time(nullptr);
			char            str[80];
			bzero(str, 80);

			if (std::strftime(str, 80, "%T", std::localtime(&t)) == 0)
				return "Error.";
			return std::string(str);
		}

		bool	channelExist(std::string	name)
		{
			for (int i = 0; i < _chV.size(); i++)
			{
				if (name.compare(_chV[i]->getName()) == 0)
					return true;
			}
			return false;
		}

		const std::string & getPassword(void) const { return this->Password; };


		// Anything else in the Server.cpp file

		void	launch();

		Client & findClient(int fd) const;
		Channel * findChannel(std::string name) const;

		void	sendChannelInformation(Client *c, Channel *ch, int id);

		void	closeClientConnection(int fd, fd_set *currentsocket);

		//check login information
		int		checkNick(std::string nick, int fd);
		int		checkUser(std::string user, int fd);

		void	login(Client *c, int event_fd, std::vector<std::string> v);
		void	Replyer(int cmd, Client *c, Message *mess, fd_set *currentsockets);
		void	MessageHandler(Message *mess, Client *c, fd_set *currentsockets);
		std::string	ReplyCreator(Message *mess, Client *c, int i);

		void	joinCmd(Message *mess, Client *c);

	private :
		int	port;
		std::string Password;
		std::vector<Client*> _cVec;
		std::vector<Channel*> _chV;


		std::string	CreationTime;
		
};

#endif