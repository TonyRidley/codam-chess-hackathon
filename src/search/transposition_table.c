#include "transposition_table.h"
#include <search.h>
#include <stdlib.h>
#include <string.h>

static int transposition_table_generation = 0;
static struct transposition_table_entry *transposition_table = NULL;

void init_transposition_table(void) {
    if (transposition_table == NULL) {
        transposition_table = (struct transposition_table_entry *)calloc(
            TRANSPOSITION_TABLE_SIZE, sizeof(struct transposition_table_entry));
    }
}

void clear_transposition_table(void) {
    if (transposition_table != NULL) {
        memset(transposition_table, 0,
               TRANSPOSITION_TABLE_SIZE * sizeof(struct transposition_table_entry));
    }
}

void new_search(void) {
    // Increment generation to invalidate older entries
    transposition_table_generation++;
}

int probe_transposition_table(const struct position *pos, int depth, int *score,
                              struct move *best_move, int alpha, int beta) {
    uint64_t current_hash = get_hash(pos);
    struct transposition_table_entry *entry =
        &transposition_table[current_hash & (TRANSPOSITION_TABLE_SIZE - 1)];

    // Check if this is the same position, same generation, and depth is sufficient
    if (entry->hash == current_hash
        && entry->generation == transposition_table_generation
        && entry->depth >= depth) {

        *best_move = entry->best_move;
        *score = entry->score;

        switch (entry->type) {
            case EXACT:
                // Exact evaluation found, immediately return it
                return 1;
            case ALPHA:
                // Upper bound
                if (entry->score <= alpha) {
                    return 1;
                }
                break;
            case BETA:
                // Lower bound
                if (entry->score >= beta) {
                    return 1;
                }
                break;
        }
    }
    return 0;
}

void store_transposition_table_entry(const struct position *pos, int depth,
                                     enum NodeType type, int score, struct move best_move) {
    uint64_t current_hash = get_hash(pos);
    struct transposition_table_entry *entry =
        &transposition_table[current_hash & (TRANSPOSITION_TABLE_SIZE - 1)];

    // Prefer storing if:
    // 1) The slot is unused or has a different hash
    // 2) The slot is from an older generation
    // 3) The slot has a smaller search depth than the one we just calculated
    if ((entry->hash != current_hash)
        || (entry->generation != transposition_table_generation)
        || (depth >= entry->depth)) {

        entry->hash = current_hash;
        entry->depth = depth;
        entry->type = type;
        entry->score = score;
        entry->best_move = best_move;
        entry->generation = transposition_table_generation;
    }
}

void free_transposition_table(void) {
    free(transposition_table);
    transposition_table = NULL;
}
