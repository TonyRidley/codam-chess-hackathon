#include <stdbool.h>

#include "bitboards.h"
#include "position.h"
#include "types.h"
#include "score.h"

int evaluate_rook(const struct position *pos)
{
	int score = 0;
	Bitboard white_pawns = pos->bitboards[WHITE][PAWN];
	Bitboard black_pawns = pos->bitboards[BLACK][PAWN];

	Bitboard column = 0x0101010101010101ULL;

	Bitboard white_rooks = pos->bitboards[WHITE][ROOK];
	Bitboard black_rooks = pos->bitboards[BLACK][ROOK];

	for (int file = 0; file < 8; file++)
	{
		Bitboard file_mask = column << file;
		bool file_has_white_pawns = (white_pawns & file_mask) != 0;
		bool file_has_black_pawns = (black_pawns & file_mask) != 0;

		// White rooks
		if (white_rooks & file_mask)
		{
			if (!file_has_white_pawns && !file_has_black_pawns)
				score += ROOK_OPEN_FILE_BONUS;
			else if (!file_has_white_pawns)
				score += ROOK_SEMI_OPEN_FILE_BONUS;
		}

		// Black rooks
		if (black_rooks & file_mask)
		{
			if (!file_has_white_pawns && !file_has_black_pawns)
				score -= ROOK_OPEN_FILE_BONUS;
			else if (!file_has_black_pawns)
				score -= ROOK_SEMI_OPEN_FILE_BONUS;
		}
	}
	return score;
}
