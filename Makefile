NAME = webserv
PROJ = Webserv

SRC_DIRS = src src/util src/http src/parse src/buffer src/response src/cgi
VPATH = $(SRC_DIRS)
OBJ_DIR = obj
INC_DIR = inc

CC = c++

OBJ = main.o Logger.o HTTPMethod.o FieldSection.o \
URI.o HTTPBody.o RequestParser.o HTTPRequestBuffer.o ReadNetBuffer.o \
RequestValidator.o ParsingUtil.o StringUtil.o ElementParser.o \
HTTPError.o Status.o RequestManager.o SysBuffer.o SysBufferFactory.o \
SysFileBuffer.o SysNetBuffer.o MediaType.o HTTPRequest.o ResponseManager.o \
File.o HTTPResponseBuffer.o VirtualServersManager.o Wspoll.o \
Parsed.o Listen.o Utils.o Location.o ServerValidator.o \
Client.o DebugView.o ConfigInheritance.o ServerConfig.o \
CGIArg.o CGI.o CGIEnv.o CGIInterpreter.o CGIResponse.o

OBJ_PATH = $(addprefix $(OBJ_DIR)/, $(OBJ))
DEPS = $(OBJ_PATH:.o=.d)

CXXFLAGS = -std=c++98 -Wall -Wextra -Werror -Wimplicit-fallthrough=5 -I$(INC_DIR)
LDFLAGS =

YELLOW = "\e[33m"
NO_COLOR = "\e[0m"

all: $(OBJ_DIR) $(NAME)

debug: CXXFLAGS += -g3 -DDEBUG_MODE=1
debug: fclean all

$(NAME): $(OBJ_PATH)
	@$(CC) $(OBJ_PATH) $(LDFLAGS) -o $(NAME)
	@echo $(YELLOW)$(PROJ) - Linking exec:$(NO_COLOR) $(NAME)

$(OBJ_DIR)/%.o: %.cpp Makefile
	@$(CC) -MMD -MP $(CXXFLAGS) -c $< -o $@
	@echo $(YELLOW)$(PROJ) - Compiling object file:$(NO_COLOR) $(notdir $@)

-include $(DEPS)

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

clean:
	@rm -rf $(OBJ_DIR)
	@echo $(YELLOW)$(PROJ) - Removing:$(NO_COLOR) Object and dependency files

fclean: clean
	@rm -f $(NAME)
	@echo $(YELLOW)$(PROJ) - Removing:$(NO_COLOR) $(NAME)

re: fclean all

.PHONY: all clean fclean re debug