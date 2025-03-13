# Executable
CHESS = the_gambit

# Compiler and flags
COMPILER = cc
CFLAGS = -Wall -Wextra -pedantic
INCLUDES = -Iincludes

# Chess engine source files
CHESS_SRC = src/evaluate/evaluate.c src/evaluate/game_phase.c src/evaluate/piece_square_value.c \
            src/search/search.c src/bitboards.c src/evaluate/pawn_structure.c \
            src/generate.c src/main.c src/move.c src/parse.c src/evaluate/rook.c \
            src/perft.c src/position.c src/uci.c src/evaluate/endgame.c src/search/opening_book.c \
            src/evaluate/mobility.c

# Objects
OBJ_DIR = obj
CHESS_OBJS = $(CHESS_SRC:%.c=$(OBJ_DIR)/%.o)

# Rules
all: $(CHESS)

$(CHESS): $(CHESS_OBJS)
	$(COMPILER) $(CHESS_OBJS) -o $(CHESS)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(COMPILER) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(CHESS)

re: fclean all

rebook: fclean book

.PHONY: all book clean fclean re rebook