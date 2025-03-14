#include "search.h"
#include "evaluate.h"
#include "generate.h"
#include "types.h"
#include "transposition_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

static volatile int time_is_up = 0;
static long start_time = 0;
static long time_limit = 0;
static int history[2][64][64];

static long get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}


static int score_move(const struct position *pos, struct move move) {
    int score = 0;
    int piece = pos->board[move.from_square];
    int captured = pos->board[move.to_square];

    if (captured != NO_PIECE) {
        score = 10 * piece_value[TYPE(captured)] - piece_value[TYPE(piece)];
    }
    if (move.promotion_type != NO_TYPE) {
        score += piece_value[move.promotion_type];
    }
    score += history[pos->side_to_move][move.from_square][move.to_square];
    return score;
}

static void order_moves(const struct position *pos, struct move *moves, size_t count) {
    for (size_t i = 0; i < count - 1; i++) {
        for (size_t j = 0; j < count - i - 1; j++) {
            if (score_move(pos, moves[j]) < score_move(pos, moves[j+1])) {
                struct move temp = moves[j];
                moves[j] = moves[j+1];
                moves[j+1] = temp;
            }
        }
    }
}

struct search_result minimax(const struct position *pos, int depth, int alpha, int beta) {
    struct search_result result;
    result.score = -1000000;
    result.move.from_square = 0;
    result.move.to_square = 0;

    // Check time every 1024 nodes
    static int nodes = 0;
    if ((++nodes & 1023) == 0) {
        if (get_time_ms() - start_time >= time_limit) {
            time_is_up = 1;
            return result;
        }
    }

    if (depth == 0) {
        result.score = evaluate(pos);
        return result;
    }

    // Transposition table lookup
    int tt_score;
    struct move tt_move = {0};
    if (probe_transposition_table(pos, depth, &tt_score, &tt_move, alpha, beta)) {
        result.score = tt_score;
        result.move = tt_move;
        return result;
    }

    struct move moves[MAX_MOVES];
    size_t count = generate_legal_moves(pos, moves);

    if (count == 0) {
        result.score = -900000;
        return result;
    }

    // Put transposition table move first if available
    if (tt_move.from_square != 0 || tt_move.to_square != 0) {
        for (size_t i = 0; i < count; i++) {
            if (moves[i].from_square == tt_move.from_square && 
                moves[i].to_square == tt_move.to_square) {
                struct move temp = moves[0];
                moves[0] = moves[i];
                moves[i] = temp;
                break;
            }
        }
    }

    order_moves(pos, moves, count);
    int original_alpha = alpha;

    for (size_t i = 0; i < count; i++) {
        struct position new_pos = *pos;
        do_move(&new_pos, moves[i]);
        
        int score = -minimax(&new_pos, depth - 1, -beta, -alpha).score;

        if (score > result.score) {
            result.score = score;
            result.move = moves[i];

            if (score > alpha) {
                alpha = score;
                if (alpha >= beta) {
                    // Store beta cutoff in transposition table
                    store_transposition_table_entry(pos, depth, LOWER, score, moves[i]);
                    break;
                }
            }
        }
    }

    // Store position in transposition table
    enum NodeType node_type;
    if (result.score <= original_alpha) {
        node_type = UPPER;
    } else if (result.score >= beta) {
        node_type = LOWER;
    } else {
        node_type = EXACT;
    }
    store_transposition_table_entry(pos, depth, node_type, result.score, result.move);

    return result;
}

struct move search(const struct search_info *info) {
    static int ply = 0;
    time_is_up = 0;
    
    // Initialize transposition table if needed
    init_transposition_table();
    
    if (info->book && ply < 20) {
        struct move book_move = get_book_move(info->pos, info->book, ply);
        if (book_move.from_square && book_move.to_square) {
            ply++;
            return book_move;
        }
    }

    struct search_result best_result = {0};
    const int max_depth = 12;
    
    start_time = get_time_ms();
    time_limit = 1000;

    // Iterative deepening
    for (int depth = 1; depth <= max_depth; depth++) {
        new_search(); // Mark new search iteration
        struct search_result result = minimax(info->pos, depth, -1000000, 1000000);
        
        if (!time_is_up) {
            best_result = result;
        }

        // Check if we're out of time
        if (get_time_ms() - start_time >= time_limit) {
            break;
        }
    }

    ply++;
    return best_result.move;
}
