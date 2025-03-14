#include "evaluate.h"

#include <generate.h>

#include "types.h"
#include "bitboards.h"
#include "position.h"

int is_king_in_check(const struct position *pos, int side);
int is_checkmate(const struct position *pos);

const int piece_value[6] = { 100, 320, 330, 500, 900, 20000 };

int evaluate(const struct position *pos)
{
	if (is_checkmate(pos))
	{
		printf("Sees checkmate!\n");
		return (pos->side_to_move == WHITE) ? -100000 : 100000;
	}

	int score[2] = { 0, 0 };
	int square;

	for (square = 0; square < 64; square++)
	{
		int piece = pos->board[square];

		if (piece != NO_PIECE)
		{
			int color = COLOR(piece);
			int type = TYPE(piece);

			// Material value
			score[color] += piece_value[type];

			// Piece square table value
			score[color] += get_piece_square_value(piece, square, get_game_phase(pos));
		}
	}
	int positional_score = 0;

	positional_score += evaluate_pawn_structure(pos);
	positional_score += evaluate_rook(pos);

	int final_score = (score[WHITE] - score[BLACK]) + positional_score;
	return (pos->side_to_move == WHITE) ? final_score : -final_score;
}

int is_checkmate(const struct position *pos) {
	struct move moves[MAX_MOVES];
    
	// First, check if we have any legal moves
	size_t legal_move_count = generate_legal_moves(pos, moves);
    
	if (legal_move_count > 0) {
		return 0; // Not checkmate if we have any legal moves
	}
    
	int king_square = -1;
	int king_piece = PIECE(pos->side_to_move, KING);
    
	// Find our king
	for (int square = 0; square < 64; square++) {
		if (pos->board[square] == king_piece) {
			king_square = square;
			break;
		}
	}
    
	// Make a "null move" - moving king to same square
	struct move null_move = make_move(king_square, king_square, NO_TYPE);
	
	return !is_legal(pos, null_move);
}

