#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include "main.hpp"
#include <vector>

class Channel
{
    public :
        Channel(std::string name) : _name(name), secret(false) {};
        Channel(std::string name, std::string key) : _name(name), _key(key), secret(false) {}; 
        ~Channel() {};


        
        void    setNewClient(int fd)
        {
            _fd.push_back(fd);
        }

        void    removeClient(int fd)
        {
            for (std::vector<int>::iterator it = _fd.begin(); it != _fd.end(); it++)
            {
                if (*it == fd)
                    _fd.erase(it);
            }
        }

        void    setNewOperator(int fd)
        {
            operators.push_back(fd);
        }

        void    removeOperator(int fd)
        {
            for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); it++)
            {
                if (*it == fd)
                    operators.erase(it);
            }
        }

        bool    isAnOperator(int fd)
        {
            for (std::vector<int>::iterator it = operators.begin(); it != operators.end(); it++)
            {
                if (*it == fd)
                    return true;
            }
            return false;
        }

        void    setSecret(bool is) { secret = is; }
        void    setTopic(std::string top) { topic = top; }

        const std::vector<int> & getClients() const { return _fd; }

        const std::string & getName() const { return _name; }

        const std::string & getKey() {return _key;}

        const std::string & getTopic() {return topic;}

        bool isSecret() {return secret;}

    private :
        std::string _name;
        std::vector<int> _fd;
        std::string _key;
        std::string topic;

        bool secret;

        std::vector<int> operators;
};


#endif
