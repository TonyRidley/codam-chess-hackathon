#include "evaluate.h"
#include "types.h"

const int piece_value[6] = { 100, 320, 330, 500, 900, 20000 };

int evaluate(const struct position *pos)
{
	int score[2] = { 0, 0 };
	int square;
	int game_phase = get_game_phase(pos);

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
			score[color] += get_piece_square_value(piece, square, game_phase);
		}
	}
	int positional_score = 0;
	int end_game_score = 0;

	positional_score += evaluate_pawn_structure(pos);
	positional_score += evaluate_rook(pos);

	end_game_score += evaluate_king_corner_endgame(pos, game_phase);
	int final_score = (score[WHITE] - score[BLACK]) + positional_score + end_game_score;
	return (pos->side_to_move == WHITE) ? final_score : -final_score;
}
