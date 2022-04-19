#ifndef MAIN_HPP
#define MAIN_HPP

#include "Server.hpp"
#include <locale>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sstream>
#include <netdb.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cstdlib>
#include <csignal>


const std::vector<std::string> ft_split(char* str, const char *ctos);


#endif
