VPATH = src src/util src/http src/parse src/buffer src/cgi

# Files
OBJ = main.o Logger.o HTTPRequest.o HTTPMethod.o FieldSection.o \
	URI.o HTTPBody.o  RequestParser.o HTTPRequestBuffer.o ReadNetBuffer.o \
	RequestValidator.o ParsingUtil.o StringUtil.o ElementParser.o \
<<<<<<< HEAD
	HTTPError.o ErrorContainer.o CGI.o CGIUtils.o
=======
	HTTPError.o Status.o RequestManager.o SysBuffer.o SysBufferFactory.o \
	SysFileBuffer.o SysNetBuffer.o debug.o serversInit.o Listen.o \
	Locations.o Parsed.o Servers.o VirtualServersManager.o ServerValidator.o \
	Utils.o serversRun.o Connection.o  \
>>>>>>> 0a6afc13775c6b82b4a5679b32370c73a186a006

# Target
NAME = webserv

# Project
PROJ = Webserv

# Directories
OBJ_DIR = obj

INC_DIR = inc

OBJ_PATH = $(addprefix $(OBJ_DIR)/, $(OBJ))

DEPS = $(OBJ_PATH:.o=.d)

# Include
INCLUDES = -I./$(INC_DIR)

# Flags -Wall -Wextra -Werror
FLAGS = -std=c++98


# Compiler
CC = c++

# Colors
YELLOW = "\e[33m"
GREEN = "\e[32m"
NO_COLOR = "\e[0m"

# Linking
all: $(OBJ_DIR) $(NAME)

$(NAME): $(OBJ_PATH) Makefile
	@$(CC) $(FLAGS) $(OBJ_PATH) -o $(NAME)
	@echo $(YELLOW)$(PROJ) - Creating exec:$(NO_COLOR) $(NAME)

# Compilation
$(OBJ_DIR)/%.o:%.cpp
	@$(CC) -MMD $(INCLUDES) $(FLAGS) -c $< -o $@
	@echo $(YELLOW)$(PROJ) - Compiling object file:$(NO_COLOR) $(notdir $@)

# Utils
-include $(DEPS)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@rm -rf $(OBJ_DIR)
	@echo $(YELLOW)$(PROJ) - Removing:$(NO_COLOR) Object and dependency files

fclean: clean
	@rm -rf $(NAME) $(NAME_B)
	@echo $(YELLOW)$(PROJ) - Removing:$(NO_COLOR) $(NAME) $(NAME_B)

re: fclean all

.PHONY: clean fclean all re $(OBJ_DIR)
