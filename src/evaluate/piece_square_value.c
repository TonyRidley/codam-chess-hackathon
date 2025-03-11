#include "types.h"
#include "evaluate.h"
#include "piece_square_value.h"

int get_piece_square_value(int piece, int square, float phase_pct)
{
	int type = TYPE(piece);
	int color = COLOR(piece);
	int sq = (color == WHITE) ? 63 - square : square;

	switch (type)
	{
		case PAWN:
		{
			int mg_value = pawn_position_start[sq];
			int eg_value = pawn_position_end[sq];
			// mg stands for middle game
			// eg stands for end game
			return (int)(mg_value * phase_pct + eg_value * (1.0f - phase_pct));
		}
		case KNIGHT:
		{
			int mg_value = knight_position_start[sq];
			int eg_value = knight_position_end[sq];
			return (int)(mg_value * phase_pct + eg_value * (1.0f - phase_pct));
		}
		case BISHOP:
		{
			int mg_value = bishop_position_start[sq];
			int eg_value = bishop_position_end[sq];
			return (int)(mg_value * phase_pct + eg_value * (1.0f - phase_pct));
		}
		case ROOK:
		{
			int mg_value = rook_position_start[sq];
			int eg_value = rook_position_end[sq];
			return (int)(mg_value * phase_pct + eg_value * (1.0f - phase_pct));
		}
		case QUEEN:
		{
			int mg_value = queen_position_start[sq];
			int eg_value = queen_position_end[sq];
			return (int)(mg_value * phase_pct + eg_value * (1.0f - phase_pct));
		}
		case KING:
		{
			int mg_value = king_position_start[sq];
			int eg_value = king_position_end[sq];
			return (int)(mg_value * phase_pct + eg_value * (1.0f - phase_pct));
		}
		default:
			return 0;
	}
}
