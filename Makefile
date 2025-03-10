# Executable
NAME = the_gambit

# Flags
COMPILER = cc
CFLAGS = -Wall -Wextra -pedantic -std=c89 -Iincludes
# CFLAGS := -Wall -Wextra -pedantic -std=c89 -O3 -flto -march=native

# Source files
SRC =	src/evaluate/evaluate.c \
		src/search/search.c \
		src/generate.c src/main.c src/move.c src/parse.c \
		src/perft.c src/position.c src/uci.c

# Objects
OBJ_DIR = obj
OBJS = $(SRC:%.c=$(OBJ_DIR)/%.o)

# Rules
all: $(NAME)

$(NAME): $(OBJS)
	$(COMPILER) $(CFLAGS) $(OBJS) -o $(NAME)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(COMPILER) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
