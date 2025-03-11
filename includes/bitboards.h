#ifndef BITBOARDS_H
#define BITBOARDS_H

#include "move.h"
#include "position.h"

#define SET_BIT(bb, square) ((bb) |= (1ULL << (square)))
#define CLEAR_BIT(bb, square) ((bb) &= ~(1ULL << (square)))
#define GET_BIT(bb, square) ((bb) & (1ULL << (square)))

void init_bitboards(struct position *pos);
void verify_board_state(const struct position *pos);
void update_bitboards_with_move(struct move move, struct position *pos);

#endif
