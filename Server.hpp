#ifndef SERVER_HPP
# define SERVER_HPP


#include "Client.hpp"
#include "Channel.hpp"
struct Message;
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

		Client * findClient(std::string name) const
		{
			for (int i = 0; i < _cVec.size(); i++)
			{
				if (name.compare(_cVec[i]->getNick()) == 0)
					return _cVec[i];
			}
			return NULL;
		}

		const std::string & getPassword(void) const { return this->Password; };
		
		// Anything else in the Server.cpp file

		void	launch();

		Client * findClient(int fd) const;
		Client * findClient(std::string fn, int x) const
		{
			x = 1;
			for (int i = 0; i < _cVec.size(); i++)
			{
				if (!_cVec[i]->getFullIdentifier().compare(fn))
					return _cVec[i];
			}
			return NULL;
		}

		Channel * findChannel(std::string name) const;

		void	sendChannelInformation(Client *c, Channel *ch);

		void	closeClientConnection(int fd, fd_set *currentsocket);

		void	deleteChannel(std::string name)
		{
			int i = 0;
			for (std::vector<Channel*>::const_iterator it = _chV.begin(); it != _chV.end(); it++)
			{
				if (_chV[i]->getName().compare(name) == 0)
				{
					delete *it;
					_chV.erase(it);
				}
				if (it == _chV.end())
					break;
			}
		}

		bool	isServerOp(int fd) const
		{
			for (std::vector<int>::const_iterator it = serverOperator.begin(); it != serverOperator.end(); it++)
			{
				if (*it == fd)
					return true;
			}
			return false;
		}

		void	removeServerOp(int fd)
		{
			for (std::vector<int>::iterator it = serverOperator.begin(); it != serverOperator.end(); it++)
			{
				if (*it == fd)
					serverOperator.erase(it);
				if (it == serverOperator.end())
					break;
			}
		}

		void	addServerOp(int fd)
		{
			serverOperator.push_back(fd);
		}

		//check
		int		checkNick(std::string nick, int fd);
		int		checkUser(std::string user, int fd);
		int		checkModes(char c);
		void	switchMode(char m, std::string sign, std::vector<std::string> mp, int* pi, Channel *ch, Client *c);

		void	login(Client *c, int event_fd, std::vector<std::string> v);
		void	Replyer(int cmd, Client *c, Message *mess, fd_set *currentsockets);
		void	MessageHandler(Message *mess, Client *c, fd_set *currentsockets);
		std::string	ReplyCreator(Message *mess, Client *c, int i);

		//CMD
		void	joinCmd(Message *mess, Client *c);
		void	privmsgCmd(Message *mess, Client *c);
		void	partCmd(Message *mess, Client *c);
		void	topicCmd(Message *mess, Client *c);
		void	quitCmd(Message *mess, Client *c, fd_set *currentsockets);
		void	nickCmd(Message *mess, Client *c);
		void	namesCmd(Message *mess, Client *c);
		void	listCmd(Message *mess, Client *c);
		void	modeCmd(Message *mess, Client *c);
		void	operCmd(Message *mess, Client *c);
		void	inviteCmd(Message *mess, Client *c);
		void	kickCmd(Message *mess, Client *c);



		void	broadcastToChan(Channel *ch, std::string msg, Client *c, bool excludeMe)
		{
			for (std::vector<int>::const_iterator it = ch->getClients().begin(); it != ch->getClients().end(); it++)
			{
				if (*it != c->getFd() || !excludeMe)
					send(*it, msg.c_str(), msg.size(), 0);
			}
		}

	private :
		int	port;
		std::string Password;
		std::vector<Client*> _cVec;
		std::vector<Channel*> _chV;

		std::vector<int> serverOperator;


		std::string	CreationTime;
		
};

#endif