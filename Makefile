

VPATH = src src/util src/http src/parse src/buffer src/response

# Files
OBJ = main.o Logger.o  HTTPMethod.o FieldSection.o \
	URI.o HTTPBody.o  RequestParser.o HTTPRequestBuffer.o ReadNetBuffer.o \
	RequestValidator.o ParsingUtil.o StringUtil.o ElementParser.o \
	HTTPError.o Status.o RequestManager.o SysBuffer.o SysBufferFactory.o \
	SysFileBuffer.o SysNetBuffer.o MediaType.o HTTPRequest.o ResponseManager.o \
	File.o HTTPResponseBuffer.o Server.o VirtualServersManager.o \
       Parsed.o Listen.o Utils.o Location.o ServerValidator.o \
	Client.o ConfigInheritance.o ServerConfig.o DebugView.o \

# Target
NAME = webserv

# Project
PROJ = Webserv

# Directories
OBJ_DIR = obj

# Source files
SRCS = main.cpp \
       Server.cpp \
       VirtualServersManager.cpp \
       Parsed.cpp \
       Listen.cpp \
       Utils.cpp \
       Location.cpp \
       ServerValidator.cpp \
       File.cpp \
       MediaType.cpp \
       http/HTTPRequest.cpp \
       http/HTTPBody.cpp \
       http/HTTPError.cpp \
       http/HTTPMethod.cpp \
       http/FieldSection.cpp \
       http/URI.cpp \
       http/RequestValidator.cpp \
       response/ResponseManager.cpp \
       parse/ElementParser.cpp \
       parse/RequestManager.cpp \
       parse/RequestParser.cpp \
       buffer/HTTPRequestBuffer.cpp \
       buffer/ReadNetBuffer.cpp \
       buffer/SysBuffer.cpp \
       buffer/SysBufferFactory.cpp \
       buffer/SysFileBuffer.cpp \
       buffer/SysNetBuffer.cpp \
       buffer/HTTPResponseBuffer.cpp\
       util/Logger.cpp \
       util/Status.cpp \
       util/StringUtil.cpp \
       util/ParsingUtil.cpp \
       



# Add src/ prefix and generate object file paths
SRC_FILES = $(addprefix $(SRC_DIR)/, $(SRCS))
OBJS = $(addprefix $(OBJ_DIR)/, $(SRCS:.cpp=.o))

# Rules
all: $(NAME)

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
	@echo $(YELLOW)$(PROJ) - Rem	oving:$(NO_COLOR) $(NAME) $(NAME_B)

re: fclean all

test: $(NAME)
	./$(NAME)

interactive: $(NAME)
	./$(NAME) --interactive

.PHONY: all clean fclean re test interactive