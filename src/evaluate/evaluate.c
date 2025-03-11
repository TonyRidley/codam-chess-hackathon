#include "evaluate.h"
#include "types.h"

const int piece_value[6] = { 100, 320, 330, 500, 900, 20000 };

int evaluate(const struct position *pos)
{
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

	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}
