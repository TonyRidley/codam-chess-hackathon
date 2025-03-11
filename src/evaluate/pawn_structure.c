#include <position.h>
#include <types.h>
#include "score.h"

int evaluate_pawn_structure(const struct position *pos)
{
	int score = 0;
	Bitboard white_pawns = pos->bitboards[WHITE][PAWN];
	Bitboard black_pawns = pos->bitboards[BLACK][PAWN];

	// Pre-compute full mask for shifting
	Bitboard column = 0x0101010101010101ULL;

	for (int file = 0; file < 8; file++)
	{
		Bitboard file_mask = column << file;

		// Doubled pawns
		int white_count = __builtin_popcountll(white_pawns & file_mask);
		int black_count = __builtin_popcountll(black_pawns & file_mask);

		if (white_count > 1)
			score -= DOUBLE_PAWN_SCORE * (white_count - 1);
		if (black_count > 1)
			score += DOUBLE_PAWN_SCORE * (black_count - 1);

		// Isolated pawns
		Bitboard adjacent = 0;
		if (file > 0) adjacent |= column << (file - 1);
		if (file < 7) adjacent |= column << (file + 1);

		if ((white_pawns & file_mask) && !(white_pawns & adjacent))
			score -= ISOLATED_PAWN_SCORE;
		if ((black_pawns & file_mask) && !(black_pawns & adjacent))
			score += ISOLATED_PAWN_SCORE;
	}
	return score;
}
