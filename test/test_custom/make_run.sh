#!/bin/bash

# Agregar proteccion para falta de argumentos
# Modificar para que no use wildcards sino listado explícito de ficheros

# Guardar argumento (target make)
ARG="$1"

# 1. Renombrar Makefile original
mv ../../Makefile ../../Makeold

# 2. Crear nuevo Makefile temporal
cat > ../../Makefile <<'EOF'
NAME = webserv

SRCS_DIR = src
OBJS_DIR = objs
INCS_DIR = inc

SRCS = \$(shell find \$(SRCS_DIR) -name "*.cpp")
OBJS = \$(SRCS:\$(SRCS_DIR)/%.cpp=\$(OBJS_DIR)/%.o)
DEPS = \$(OBJS:.o=.d)

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98 -I\$(INCS_DIR)
DEPFLAGS = -MMD -MP

all: \$(NAME)

\$(NAME): \$(OBJS)
	@echo "Linking \$(NAME)..."
	@\$(CXX) \$(CXXFLAGS) \$^ -o \$@
	@echo "\$(NAME) created successfully!"

\$(OBJS_DIR)/%.o: \$(SRCS_DIR)/%.cpp
	@mkdir -p \$(dir \$@)
	@echo "Compiling \$<..."
	@\$(CXX) \$(CXXFLAGS) \$(DEPFLAGS) -c \$< -o \$@

clean:
	@echo "Cleaning objects..."
	@rm -rf \$(OBJS_DIR)

fclean: clean
	@echo "Cleaning \$(NAME)..."
	@rm -f \$(NAME)
	@rm -rf logs/*.log

re: fclean all

test: all
	@echo "Setting up test environment..."
	@bash tests/setup_test_env.sh
	@echo "Running tests..."
	@bash tests/run_all_tests.sh

valgrind: all
	valgrind --leak-check=full --show-leak-kinds=all \
	         --track-origins=yes --verbose \
	         ./\$(NAME) conf/webserv.conf

.PHONY: all clean fclean re test valgrind

-include \$(DEPS)
EOF

# 3. Ejecutar el Makefile temporal con argumento
make -C ../.. $ARG

# 4. Copiar ejecutable webserv a la carpeta actual
cp -f ../../webserv .

# 5. Borrar Makefile temporal
rm -f ../../Makefile

# 6. Restaurar Makefile original
mv ../../Makeold ../../Makefile
