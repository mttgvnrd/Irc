# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: mgiovana <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/09/30 12:12:21 by mgiovana          #+#    #+#              #
#    Updated: 2024/09/30 12:13:50 by mgiovana         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = ircserv
CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98
RM = rm -rf
SRC_F = src/
OBJ_F = obj/
SRC = main.cpp Server.cpp Client.cpp Channel.cpp
OBJ = $(SRC:.cpp=.o)
OBJ := $(addprefix $(OBJ_F),$(OBJ))

$(OBJ_F)%.o : $(SRC_F)%.cpp
		mkdir -p $(OBJ_F)
		$(CC) $(FLAGS) -c $< -o $@

$(NAME): $(OBJ)
		$(CC) $(FLAGS) $(OBJ) -o $(NAME)

all: $(NAME)

%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
		$(RM) $(OBJ)
		$(RM) $(OBJ_F)

fclean: clean
		$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re

