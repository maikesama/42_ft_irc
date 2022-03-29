#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "main.hpp"
#include <iostream>

class Client 
{

    public :
        Client(int fd, std::string Host) : _fd(fd), _HostAddress(Host), _isRegistered(false) {};
        ~Client() {};

        bool    getIsRegistered() const { return _isRegistered; };
        int     getFd() const { return _fd; };
        const std::string & getNick() const { return _Nick; };
        const std::string & getRealName() const { return _Realname; };
        const std::string & getUsername() const { return _Username; };
        const std::string & getHostAddress() const { return _HostAddress; };

        void    setNick(const std::string & n) { _Nick = n; 
        //control isvalid
        }
        void    setRealName(const std::string & n) { _Realname = n; }
        void    setUsername(const std::string & n) { _Username = n; }
        void    setIsRegistered( bool k ) { _isRegistered = k; }



    private :
        std::string _Nick;
        std::string _Realname;
        std::string _Username;
        std::string _HostAddress;

        bool        _isRegistered;

        int         _fd;

};

#endif