#include "search.h"
#include "evaluate.h"
#include "generate.h"
#include "transposition_table.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static int history[2][64][64];

static int score_move(const struct position *pos, struct move move) {
	int score = 0;
	int piece = pos->board[move.from_square];
	int captured = pos->board[move.to_square];

	if (captured != NO_PIECE) {
		score = 10 * piece_value[TYPE(captured)] - piece_value[TYPE(piece)];
	}
	if (move.promotion_type != NO_TYPE) {
		score += piece_value[move.promotion_type];
	}
	score += history[pos->side_to_move][move.from_square][move.to_square];
	return score;
}

static void order_moves(const struct position *pos, struct move *moves, size_t count) {
	for (size_t i = 0; i < count - 1; i++) {
		for (size_t j = 0; j < count - i - 1; j++) {
			if (score_move(pos, moves[j]) < score_move(pos, moves[j+1])) {
				struct move temp = moves[j];
				moves[j] = moves[j+1];
				moves[j+1] = temp;
			}
		}
	}
}

struct search_result minimax(const struct position *pos, int depth, int alpha, int beta) {
	struct search_result result;
	result.score = -1000000;
	result.move.from_square = 0;
	result.move.to_square = 0;

	if (depth == 0) {
		result.score = evaluate(pos);
		return result;
	}

	struct move moves[MAX_MOVES];
	size_t count = generate_legal_moves(pos, moves);

	if (count == 0) {
		result.score = -900000;
		return result;
	}

	order_moves(pos, moves, count);

	for (size_t i = 0; i < count; i++) {
		struct position new_pos = *pos;
		do_move(&new_pos, moves[i]);
		
		int score = -minimax(&new_pos, depth - 1, -beta, -alpha).score;

		if (score > result.score) {
			result.score = score;
			result.move = moves[i];

			if (score > alpha) {
				alpha = score;
				if (alpha >= beta) {
					break;
				}
			}
		}
	}

	return result;
}

struct move search(struct search_info *info) {
	static int ply = 0;

	// Try book moves first (if we're in early game)
	if (info->book && ply < 20) {
		struct move book_move = get_book_move(info->pos, info->book, ply);
		if (book_move.from_square && book_move.to_square) {
			ply++;
			return book_move;
		}
	}

	struct search_result best_result = {0};
	const int max_depth = 8;
	time_t start_time = time(NULL);

	// Iterative deepening
	for (int depth = 1; depth <= max_depth; depth++) {
		struct search_result result = minimax(info->pos, depth, -1000000, 1000000);
		best_result = result;

		// Time management
		time_t current_time = time(NULL);
		if ((int)(current_time - start_time) >= 1) { 
			break;
		}
	}
	ply++;
	return best_result.move;
}


