#include <position.h>
#include <types.h>
#include "score.h"

int evaluate_pawn_structure(const struct position *pos)
{
	int score = 0;
	Bitboard white_pawns = pos->bitboards[WHITE][PAWN];
	Bitboard black_pawns = pos->bitboards[BLACK][PAWN];

	Bitboard column = 0x0101010101010101ULL;

	for (int file = 0; file < 8; file++)
	{
		Bitboard file_mask = column << file;

		// Doubled pawns
		int white_count = __builtin_popcountll(white_pawns & file_mask);
		int black_count = __builtin_popcountll(black_pawns & file_mask);

		if (white_count > 1)
			score -= DOUBLE_PAWN_BONUS * (white_count - 1);
		if (black_count > 1)
			score += DOUBLE_PAWN_BONUS * (black_count - 1);

		// Isolated pawns
		Bitboard adjacent = 0;
		if (file > 0) adjacent |= column << (file - 1);
		if (file < 7) adjacent |= column << (file + 1);

		if ((white_pawns & file_mask) && !(white_pawns & adjacent))
			score -= ISOLATED_PAWN_BONUS;
		if ((black_pawns & file_mask) && !(black_pawns & adjacent))
			score += ISOLATED_PAWN_BONUS;

		// Passed pawns: White
		Bitboard white_file_pawns = white_pawns & file_mask;
		while (white_file_pawns)
		{
			int pawn_square = __builtin_ctzll(white_file_pawns);
			int rank = pawn_square / 8;

			Bitboard front_mask = file_mask;
			if (file > 0) front_mask |= (column << (file - 1));
			if (file < 7) front_mask |= (column << (file + 1));

			if (rank < 7)
				front_mask &= ~((1ULL << (rank * 8 + 8)) - 1);
			else
				front_mask = 0ULL; 

			if (!(black_pawns & front_mask))
				score += PASSED_PAWN_BONUS;

			white_file_pawns &= (white_file_pawns - 1);
		}

		// Passed pawns: Black
		Bitboard black_file_pawns = black_pawns & file_mask;
		while (black_file_pawns)
		{
			int pawn_square = __builtin_ctzll(black_file_pawns);
			int rank = pawn_square / 8;

			Bitboard front_mask = file_mask;
			if (file > 0) front_mask |= (column << (file - 1));
			if (file < 7) front_mask |= (column << (file + 1));

			if (rank > 0)
				front_mask &= ((1ULL << (rank * 8)) - 1);
			else
				front_mask = 0ULL;

			if (!(white_pawns & front_mask))
				score -= PASSED_PAWN_BONUS;

			black_file_pawns &= (black_file_pawns - 1);
		}
	}
	return score;
}
