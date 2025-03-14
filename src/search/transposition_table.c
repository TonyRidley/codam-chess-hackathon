#include "transposition_table.h"
#include <search.h>
#include <stdlib.h>
#include <string.h>

static int transposition_table_age = 0;
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
    transposition_table_age++;
}

int probe_transposition_table(const struct position *pos, int depth, int *score,
                            struct move *best_move, int alpha, int beta) {
    uint64_t current_hash = get_hash(pos);
    uint32_t index = (uint32_t)(current_hash & (TRANSPOSITION_TABLE_SIZE - 1));
    struct transposition_table_entry *entry = &transposition_table[index];

    if (entry->hash == current_hash) {
        *best_move = entry->best_move;
        
        // Adjust mate scores based on current ply
        if (entry->score > 900000) {
            *score = entry->score - 1;
        } else if (entry->score < -900000) {
            *score = entry->score + 1;
        } else {
            *score = entry->score;
        }

        if (entry->depth >= depth) {
            switch (entry->type) {
                case EXACT:
                    return 1;
                case LOWER:
                    if (entry->score >= beta) {
                        *score = beta;
                        return 1;
                    }
                    break;
                case UPPER:
                    if (entry->score <= alpha) {
                        *score = alpha;
                        return 1;
                    }
                    break;
            }
        }
    }
    return 0;
}

void store_transposition_table_entry(const struct position *pos, int depth,
                                   enum NodeType type, int score, struct move best_move) {
    uint64_t current_hash = get_hash(pos);
    uint32_t index = (uint32_t)(current_hash & (TRANSPOSITION_TABLE_SIZE - 1));
    struct transposition_table_entry *entry = &transposition_table[index];

    // Always replace if we have a deeper search or older age
    if (entry->depth <= depth || entry->age != transposition_table_age) {
        entry->hash = current_hash;
        entry->depth = depth;
        entry->type = type;
        entry->score = score;
        entry->best_move = best_move;
        entry->age = transposition_table_age;
    }
}

void free_transposition_table(void) {
    free(transposition_table);
    transposition_table = NULL;
}
