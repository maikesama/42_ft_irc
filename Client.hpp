#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "main.hpp"
#include <iostream>

class Client 
{

    public :
        Client(int fd, std::string Host) : _fd(fd), _HostAddress(Host), _isRegistered(false), first(false) {};
        Client() {};
        ~Client() {};

        const Client & operator=(const Client & c)
        {
            _Nick = c._Nick;
            _Realname = c._Realname;
            _Username = c._Username;
            _fd = c._fd;
            _HostAddress = c._HostAddress;
            _isRegistered = c._isRegistered;
            return *this;
        }

        Client(const Client & c)
        {
            *this = c;
        }

        bool    getIsRegistered() const { return _isRegistered; };
        bool    getFirst() const { return first; };
        int     getFd() const { return _fd; };
        const std::string & getNick() const { return _Nick; };
        const std::string & getRealName() const { return _Realname; };
        const std::string & getUsername() const { return _Username; };
        const std::string & getHostAddress() const { return _HostAddress; };

        void    setNick(const std::string & n) { _Nick = n; 
        //control isvalid
        }
        void    setFirst() { first = true; }
        void    setRealName(const std::string & n) { _Realname = n; }
        void    setUsername(const std::string & n) { _Username = n; }
        void    setIsRegistered( bool k ) { _isRegistered = k; }



    private :
        std::string _Nick;
        std::string _Realname;
        std::string _Username;
        std::string _HostAddress;

        bool        _isRegistered;

        bool        first;

        int         _fd;

};

#endif