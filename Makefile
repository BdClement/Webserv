## MAIN VARIABLES

NAME =			webserv

CC = 			c++

CFLAGS = 		-Wall -Wextra -Werror -std=c++98

RM =			rm -f

RM_RF =			rm -rf

GREEN = \033[0;32m

## SOURCES AND LIBS

SRC_DIR = 		./src
SRCS =			$(SRC_DIR)/main.cpp\
				$(SRC_DIR)/Handler.cpp\
				$(SRC_DIR)/Utils.cpp\
				$(SRC_DIR)/Config.cpp\
				$(SRC_DIR)/Connection.cpp\

## OBJECTS

OBJ_DIR =		./obj
OBJS = 			$(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.o, $(SRCS))

## HEADERS

HDIR = 			./inc
HD = 			$(HDIR)/webserv.hpp\
				$(HDIR)/Handler.hpp\
				$(HDIR)/Config.hpp\
				$(HDIR)/Connection.hpp\

INC_HD =		-I $(HDIR)

## RULES

all:			$(NAME)

$(NAME):		$(OBJS)
				@echo "$(GREEN)Compiling webserv program..."
				@$(CC) $(CFLAGS) $(OBJS) -o $@
				@echo "$(GREEN)Compilation done"

$(OBJS):		$(OBJ_DIR)/%.o:	$(SRC_DIR)/%.cpp $(HD)
				@mkdir -p $(OBJ_DIR)
				@$(CC) $(CFLAGS) $(INC_HD) -c $< -o $@

clean:
				@echo "$(GREEN)Cleaning webserv"
				@$(RM_RF) $(OBJ_DIR)

fclean:			clean
				@echo "$(GREEN)Removing executable file"
				@$(RM) $(NAME)

re:				fclean all

.PHONY: 		all clean fclean re
