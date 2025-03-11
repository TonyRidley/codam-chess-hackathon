#include "position.h"
#include "types.h"

int get_game_phase(const struct position *pos)
{
	int phase = 24;

	// Count knights and bishops (1 point each)
	phase -= __builtin_popcountll(pos->bitboards[WHITE][KNIGHT] | pos->bitboards[BLACK][KNIGHT]);
	phase -= __builtin_popcountll(pos->bitboards[WHITE][BISHOP] | pos->bitboards[BLACK][BISHOP]);

	// Count rooks (2 points each)
	phase -= 2 * __builtin_popcountll(pos->bitboards[WHITE][ROOK] | pos->bitboards[BLACK][ROOK]);

	// Count queens (4 points each)
	phase -= 4 * __builtin_popcountll(pos->bitboards[WHITE][QUEEN] | pos->bitboards[BLACK][QUEEN]);

	return phase;
}
