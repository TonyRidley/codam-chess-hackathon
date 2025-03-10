#include "evaluate.h"
#include "types.h"

static const int piece_value[6] = { 100, 300, 300, 500, 900, 1000000 };

int evaluate(const struct position *pos) {
	int score[2] = { 0, 0 };
	int square;

	for (square = 0; square < 64; square++) {
		int piece = pos->board[square];

		if (piece != NO_PIECE) {
			int color = COLOR(piece);
			int type = TYPE(piece);
			score[COLOR(piece)] += piece_value[TYPE(piece)];
			if (type == PAWN)
			{
				if (color == WHITE)
					score[color] += pawn_position[square];
				else
					score[color] += pawn_position[63 - square];
			}
			else if(type == BISHOP)
			{
				if (color == WHITE)
					score[color] += bishop_position[square];
				else
					score[color] += bishop_position[63 - square];
			}
			else if(type == KNIGHT)
			{
				if (color == WHITE)
					score[color] += knight_position[square];
				else
					score[color] += knight_position[63 - square];
			}
			else if (type == ROOK)
			{
				if (color == WHITE)
					score[color] += rook_position[square];
				else
					score[color] += rook_position[63 - square];
			
			}
			else if (type == QUEEN)
			{
				if (color == WHITE)
					score[color] += queen_position[square];
				else
					score[color] += queen_position[63 - square];
			}
		}
	}

	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}