#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <stdbool.h>

#include "move.h"
#include "types.h"

#define TRANSPOSITION_TABLE_SIZE (64 * 1024 * 1024)  // Size of transposition table - 64MB

enum NodeType {
	EXACT,
	UPPER,
	LOWER
};

struct transposition_table_entry {
	__uint64_t hash;      // Zobrist hash of the position
	int depth;            // Depth of the search
	enum NodeType type;   // Type of node (exact, alpha, or beta)
	int score;           // Score of the position
	struct move best_move; // Best move found for this position
	int age;
};

void init_transposition_table(void);
void clear_transposition_table(void);
void store_transposition_table_entry(const struct position *pos, int depth, enum NodeType type, int score, struct move best_move);
int probe_transposition_table(const struct position *pos, int depth, int *score, struct move *best_move, int alpha, int beta);
void new_search(void);
void free_transposition_table(void);


#endif
