#include "position.h"
#include "types.h"

int get_game_phase(const struct position *pos)
{
	// Total phase score at start = 256
	const int TOTAL_PHASE = 256;

	// Piece phase values
	const int KNIGHT_PHASE = 16;
	const int BISHOP_PHASE = 16;
	const int ROOK_PHASE = 32;
	const int QUEEN_PHASE = 64;

	int phase = TOTAL_PHASE;

	// Subtract phase value for each missing piece
	phase -= KNIGHT_PHASE * __builtin_popcountll(pos->bitboards[WHITE][KNIGHT] | pos->bitboards[BLACK][KNIGHT]);
	phase -= BISHOP_PHASE * __builtin_popcountll(pos->bitboards[WHITE][BISHOP] | pos->bitboards[BLACK][BISHOP]);
	phase -= ROOK_PHASE * __builtin_popcountll(pos->bitboards[WHITE][ROOK] | pos->bitboards[BLACK][ROOK]);
	phase -= QUEEN_PHASE * __builtin_popcountll(pos->bitboards[WHITE][QUEEN] | pos->bitboards[BLACK][QUEEN]);

	// Convert to 0-24 range for compatibility with existing code
	phase = (phase * 24) / TOTAL_PHASE;

	return phase;
}
