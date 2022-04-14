#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "main.hpp"
#include <vector>

class Channel
{
	public :
		Channel(std::string name) : _name(name) {};
		Channel(std::string name, std::string key) : _name(name), _key(key) {}; 
		~Channel() {};


		
		void	setNewClient(int fd)
		{
			_fd.push_back(fd);
		}

		void	removeClient(int fd)
		{
			for (std::vector<int>::iterator it = _fd.begin(); it != _fd.end(); it++)
			{
				if (*it == fd)
					_fd.erase(it);
				if (it == _fd.end())
					break;
			}
		}

		void	setNewOperator(int fd)
		{
			operators.push_back(fd);
		}

		void	removeOperator(int fd)
		{
			for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); it++)
			{
				if (*it == fd)
					operators.erase(it);
				if (it == operators.end())
					break;
			}
		}

		bool	isAnOperator(int fd)
		{
			if (modes.find("o") == std::string::npos)
				return true;
			for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); it++)
			{
				if (*it == fd)
					return true;
			}
			return false;
		}

		bool 	isMode(char x)
		{
			if (modes.find(x) != std::string::npos)
				return true;
			return false;
		}

		void	setKey(std::string k) {_key = k; }
		void	setTopic(std::string top) { topic = top; }
		void	setModes(std::string md)
		{
			if (md[0] == '+')
			{
				for (int i = 1; i < md.size(); i++)
				{
					if(modes.find(md[i]) == std::string::npos)
						modes += md[i];
				}
			}
			else if (md[0] == '-')
			{
				for (int i = 1; i < md.size(); i++)
				{
					if(modes.find(md[i]) != std::string::npos)
					{
						int find = modes.find(md[i]);
						modes.erase(find, 1);
					}
				}
			}
		}

		void	removeBan(const std::string & name)
		{
			for (std::vector<std::string>::iterator it = banList.begin(); it != banList.end(); it++)
			{
				if (!name.compare(*it))
					banList.erase(it);
				if (it == banList.end())
					break;
			}
		}

		bool	isBanned(const std::string & name)
		{
			for (std::vector<std::string>::iterator it = banList.begin(); it != banList.end(); it++)
			{
				if (!name.compare(*it))
					return true;
			}
			return false;
		}

		void	addBanned(const std::string & name)
		{
			banList.push_back(name);
		}

		const std::vector<int> & getClients() const { return _fd; }

		const std::string & getName() const { return _name; }

		const std::string & getModes() const { return modes; }

		const std::string & getKey() {return _key;}

		const std::string & getTopic() {return topic;}

		bool isSecret() { return isMode('s'); }

		void	CanTalk(int fd){ canTalk.push_back(fd); }
		void	CantTalk(int fd)
		{
			for (std::vector<int>::const_iterator it = canTalk.begin(); it != canTalk.end(); it++)
			{
				if (*it == fd)
					canTalk.erase(it);
				if (it == canTalk.end())
					break;
			}
		}

		bool	canITalk(int fd)
		{
			for (std::vector<int>::const_iterator it = canTalk.begin(); it != canTalk.end(); it++)
			{
				if (*it == fd)
					return true;
			}
			return false;
		}

	private :
		std::string _name;
		std::vector<int> _fd;
		std::string _key;
		std::string topic;

		std::vector<int> operators;
		std::string modes;

		std::vector<std::string> banList;
		std::vector<int> canTalk;

};


#endif
