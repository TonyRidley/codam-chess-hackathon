#include "position.h"
#include "types.h"

float get_game_phase(const struct position *pos)
{
	const int TOTAL_PHASE = 256;
	const int KNIGHT_PHASE = 16;
	const int BISHOP_PHASE = 16;
	const int ROOK_PHASE = 32;
	const int QUEEN_PHASE = 64;

	int phase = 0; // Start from 0 instead of TOTAL_PHASE

	// Add phase value for each piece present
	phase += KNIGHT_PHASE * __builtin_popcountll(pos->bitboards[WHITE][KNIGHT] | pos->bitboards[BLACK][KNIGHT]);
	phase += BISHOP_PHASE * __builtin_popcountll(pos->bitboards[WHITE][BISHOP] | pos->bitboards[BLACK][BISHOP]);
	phase += ROOK_PHASE * __builtin_popcountll(pos->bitboards[WHITE][ROOK] | pos->bitboards[BLACK][ROOK]);
	phase += QUEEN_PHASE * __builtin_popcountll(pos->bitboards[WHITE][QUEEN] | pos->bitboards[BLACK][QUEEN]);

	return (float)phase / TOTAL_PHASE;
}
