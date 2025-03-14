#include <position.h>
#include "types.h"
#include <zobrist_keys.h>
#include <types.h>

__uint64_t get_hash(const struct position *pos)
{
	__uint64_t hash = 0ULL;

	for (int square = 0; square < 64; square++)
	{
		int piece = pos->board[square];
		if (piece != NO_PIECE)
		{
			int color = COLOR(piece);
			int type = TYPE(piece);
			int color_offset = color * 6;
			int z_index = square * NUM_PIECES + (color_offset + type);
			hash ^= ZOBRIST_KEYS[z_index];
		}
	}
	return hash;
}

__uint64_t update_z_table(__uint64_t hash, int from, int to, int piece)
{
	int from_index = from * NUM_PIECES + piece;
	hash ^= ZOBRIST_KEYS[from_index];

	int to_index = to * NUM_PIECES + piece;
	hash ^= ZOBRIST_KEYS[to_index];

	return hash;
}
