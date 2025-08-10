VPATH = src src/util src/http src/parse src/buffer

# Variables
NAME = webserv_test
CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -Iinc

# Directories
SRC_DIR = src
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

$(NAME): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(NAME) $(OBJS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

test: $(NAME)
	./$(NAME)

interactive: $(NAME)
	./$(NAME) --interactive

.PHONY: all clean fclean re test interactive