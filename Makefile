# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: luigi <luigi@student.42.fr>                +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2024/07/27 09:45:13 by luigi             #+#    #+#              #
#    Updated: 2024/11/08 09:52:31 by luigi            ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME			=	./ircserv

VALGRIND-TOOL	=	memcheck

CC				=	c++
CFLAGS			=	-g
REQUIRED_CFLAGS	=	$(CFLAGS) -Wall -Wextra -Werror -std=c++98
CPPFLAGS		=	$(addprefix -I,$(INC_DIR))
LDFLAGS			=	$(addprefix -L, )
LDLIBS			=	$(addprefix -l, )

BUILD_DIR		=	build
INC_DIR			=	$(BUILD_DIR)/inc
OBJS_DIR		=	$(BUILD_DIR)/obj
SRCS_DIR		=	src

SRCS			=	$(SRCS_DIR)/main.cpp \
					$(SRCS_DIR)/Server.cpp \
					$(SRCS_DIR)/Channel.cpp \
					$(SRCS_DIR)/ClientInstance.cpp \
					$(SRCS_DIR)/Utils.cpp \
					$(SRCS_DIR)/HandleClientMsg.cpp \
					$(SRCS_DIR)/HandleErrors.cpp 

OBJS			=	$(SRCS:$(SRCS_DIR)%.cpp=$(OBJS_DIR)%.o)

RM				=	rm -fr

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) $(REQUIRED_CFLAGS) $(CPPFLAGS) $(LDFLAGS) $(LDLIBS) -o $(NAME)

$(OBJS_DIR)/%.o: $(SRCS_DIR)/%.cpp $(P_HEADER)
	@mkdir -p $(@D)
	$(CC) -c $< $(REQUIRED_CFLAGS) $(CPPFLAGS) -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

clear:
	clear

run: clear all
	$(NAME) $(ARGS)

mem: clear all
	valgrind --tool=$(VALGRIND-TOOL) $(VALGRIND-OPTIONS) $(NAME) $(ARGS)

vgdb: clear all
	valgrind --tool=$(VALGRIND-TOOL) $(VALGRIND-OPTIONS) --vgdb-error=0 $(NAME) $(ARGS)

gdb: clear all
	echo "target remote | vgdb\nc" > .gdbinit
	gdb --args $(NAME) $(ARGS)

debug: clear all
	gdb --args $(NAME) $(ARGS)

debugf: clear all
	vi .gdbinit && gdb --args $(NAME) $(ARGS)

PHONY:
	all clean fclean re clear run mem vgdb gdb debug debugf
