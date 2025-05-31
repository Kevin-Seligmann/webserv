TARGET = webserv

CXX = g++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -Iinc

SRC_DIR = src
OBJ_DIR = obj
INC_DIR = inc

SRC_FILES = main.cpp \
			ServerValidator.cpp \
			Parsed.cpp \
			Utils.cpp

SRCS = $(addprefix $(SRC_DIR)/, $(SRC_FILES))

OBJS = $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.cpp=.o))

DEPS = $(addprefix $(OBJ_DIR)/, $(SRC_FILES:.cpp=.d))

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp | $(OBJ_DIR) Makefile
	$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

-include $(DEPS)

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
