#include "evaluate.h"
#include "types.h"

static const int piece_value[6] = { 100, 300, 300, 500, 900, 1000000 };

int evaluate(const struct position *pos)
{
	int score[2] = { 0, 0 };
	int square;

	for (square = 0; square < 64; square++)
	{
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
		}
	}

	return score[pos->side_to_move] - score[1 - pos->side_to_move];
}

// static const int pawn_position[64] = {
// 	 0,  0,  0,  0,  0,  0,  0,  0,
// 	50, 50, 50, 50, 50, 50, 50, 50,
// 	10, 10, 20, 30, 30, 20, 10, 10,
//  	5,  5, 10, 25, 25, 10,  5,  5,
//  	0,  0,  0, 20, 20,  0,  0,  0,
//  	5, -5,-10,  0,  0,-10, -5,  5,
// 	 5, 10, 10,-20,-20, 10, 10,  5,
//  	0,  0,  0,  0,  0,  0,  0,  0
// };

// static const int bishop_position[64] = {
// 	-20,-10,-10,-10,-10,-10,-10,-20,
// 	-10,  0,  0,  0,  0,  0,  0,-10,
// 	-10,  0,  5, 10, 10,  5,  0,-10,
// 	-10,  5,  5, 10, 10,  5,  5,-10,
// 	-10,  0, 10, 10, 10, 10,  0,-10,
// 	-10, 10, 10, 10, 10, 10, 10,-10,
// 	-10,  5,  0,  0,  0,  0,  5,-10,
// 	-20,-10,-10,-10,-10,-10,-10,-20,
// };
