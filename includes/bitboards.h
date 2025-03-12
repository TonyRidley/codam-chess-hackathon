#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "move.h"
#include "position.h"

#define SET_BIT(bb, square) ((bb) |= (1ULL << (square)))
#define CLEAR_BIT(bb, square) ((bb) &= ~(1ULL << (square)))
#define GET_BIT(bb, square) ((bb) & (1ULL << (square)))

static inline int get_lsb(uint64_t bb)
{
	if (bb == 0) return -1;
	return __builtin_ctzll(bb);
}

void init_bitboards(struct position *pos);
void verify_board_state(const struct position *pos);
void update_bitboards_with_move(struct move move, struct position *pos);

#endif
