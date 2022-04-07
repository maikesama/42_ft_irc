NAME = ircserv

CC = g++ -Wall -Werror -Wextra -std=c++98 -fsanitize=address

SRCS = main.cpp Server.cpp Commands.cpp\

OBJS = $(SRCS:.cpp=.o)

all : $(NAME)

$(NAME) : $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

clean :
	rm -f $(NAME)

fclean :
	rm -f $(NAME) $(OBJS)

re : fclean all