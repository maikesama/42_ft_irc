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

        const std::vector<int> & getClients() const { return _fd; }

        const std::string & getName() const { return _name; }

        const std::string & getKey() {return _key;}

    private :
        std::string _name;
        std::vector<int> _fd;
        std::string _key;
};


#endif
