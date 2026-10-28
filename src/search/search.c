#include "search.h"

#include <limits.h>

#include "transposition_table.h"
#include "evaluate.h"
#include "generate.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// Example global or static arrays, if desired:
static int history[2][64][64]; // For move-ordering heuristics like the history heuristic

// ------------------------------------------------------
// Helper to score a single move (placeholder/heuristic)
// ------------------------------------------------------
static int score_move(const struct position *pos, struct move mv) {
    // Example: return a higher value for captures, promotions, etc.
    // You could incorporate history, killer moves, etc.
    // For now, just a placeholder:
    int base_score = 0;
    (void)pos; // Mark unused
    (void)mv;  // Mark unused

    // if (is_capture(pos, mv)) {
    //     // Basic MVV-LVA or something similar
    //     base_score += 100000;
    // }

    // Improve ordering further using your other heuristics:
    // e.g., if (history[color_to_move][mv.start][mv.end]) ...

    return base_score;
}

// ------------------------------------------------------
// Order moves in descending priority
// ------------------------------------------------------
static void order_moves(const struct position *pos, struct move *moves, size_t count) {
    // A simple insertion sort or similar:
    for (size_t i = 0; i < count; i++) {
        int best_index = (int) i;
        int best_score = score_move(pos, moves[i]);
        for (size_t j = i + 1; j < count; j++) {
            int scr = score_move(pos, moves[j]);
            if (scr > best_score) {
                best_index = (int) j;
                best_score = scr;
            }
        }
        if (best_index != (int) i) {
            struct move temp = moves[i];
            moves[i] = moves[best_index];
            moves[best_index] = temp;
        }
    }
}
/**
 * A simple alpha-beta minimax search using the transposition table.
 * Returns a search_result containing the best found move and its score.
 */
struct search_result minimax(const struct position *pos, int depth, int alpha, int beta) {
    struct search_result result;

    // A null move placeholder.
    // Adjust it to whatever "no move" representation you use.
    struct move null_move = {0, 0, 0};

    result.score = 0;
    result.best_move = null_move;

    // -------------------------------------------------
    // 1) Check if we reached zero depth => Evaluate
    // -------------------------------------------------
    if (depth <= 0) {
        // Return an evaluation (or call a quiescence search function here).
        result.score = evaluate(pos);
        return result;
    }

    // -------------------------------------------------
    // 2) Try probe TT
    // -------------------------------------------------
    {
        int tt_score;
        struct move tt_move = null_move;
        if (probe_transposition_table(pos, depth, &tt_score, &tt_move, alpha, beta)) {
            // If the probe is valid, we can return immediately
            result.score = tt_score;
            result.best_move = tt_move;
            return result;
        }
    }

    // -------------------------------------------------
    // 3) Generate moves
    // -------------------------------------------------
    struct move moves[256];
    size_t move_count = generate_legal_moves(pos, moves);

    // Order the moves so that good/capturing moves are tried first
    order_moves(pos, moves, move_count);

    int best_score = -INT_MAX;
    struct move best_mv = null_move;

    // -------------------------------------------------
    // 4) Loop through all moves
    // -------------------------------------------------
    for (size_t i = 0; i < move_count; i++) {
        struct move mv = moves[i];

        // Create a copy of the position to apply the move
        struct position copy = *pos;
        do_move(&copy, mv);

        // Recurse: switch perspective => negate the child's score
        struct search_result child_result = minimax(&copy, depth - 1, -beta, -alpha);
        int score = -child_result.score;

        // Check if this child is better than our current best
        if (score > best_score) {
            best_score = score;
            best_mv = mv;

            // Update alpha
            if (score > alpha) {
                alpha = score;
                // Beta cutoff
                if (alpha >= beta) {
                    break;
                }
            }
        }
    }

    result.score = best_score;
    result.best_move = best_mv;

    // -------------------------------------------------
    // 5) Store to the transposition table
    // -------------------------------------------------
    enum NodeType node_type = ALPHA; // default "fail-low"
    if (best_score >= beta) {
        node_type = BETA; // fail-high
    } else if (best_score > alpha && best_score < beta) {
        node_type = EXACT;
    }
    // Note: If best_score <= alpha, node_type = ALPHA is correct.

    store_transposition_table_entry(pos, depth, node_type, best_score, best_mv);

    return result;
}

struct move search(struct search_info *info) {
    // "ply" can keep track of how many moves have been made in the game
    static int ply = 0;

    // 1) Check Opening Book (if available and within 20 moves)
    if (info->book && ply < 20) {
        struct move book_move = get_book_move(info->pos, info->book, ply);
        // If we got a valid book move, just return it
        if (book_move.from_square && book_move.to_square) {
            ply++;
            return book_move;
        }
    }

    // 2) Initialize Transposition Table (do this once per search session)
    init_transposition_table();
    new_search();
    memset(history, 0, sizeof(history));

    // Basic search parameters: maximum depth, time limit, etc.
    const int max_depth = 12;
    const int thinking_time = 1; // in seconds
    time_t start_time = time(NULL);

    int prev_score = 0;
    struct search_result best_result;
    best_result.score = 0;
    best_result.best_move.from_square = 0;
    best_result.best_move.to_square = 0;
    best_result.best_move.promotion_type = NO_TYPE;

    // 3) Iterative Deepening Loop
    for (int depth = 1; depth <= max_depth; depth++) {
        struct search_result result;

        // A) If we don’t have a good guess from a previous iteration (or on small depths),
        //    do a normal wide-window search
        if (depth <= 4) {
            result = minimax(info->pos, depth, -50000, 50000);
        } else {
            // B) Aspiration windows: try a narrow window around prev_score
            int delta = 50;
            int alpha = prev_score - delta;
            int beta = prev_score + delta;
            result = minimax(info->pos, depth, alpha, beta);

            // If we “fail-high” or “fail-low” with the narrow window, do a full-window search
            if (result.score <= alpha || result.score >= beta) {
                result = minimax(info->pos, depth, -50000, 50000);
            }
        }

        // Keep track of best result so far
        best_result = result;
        prev_score = result.score;

        // 4) Output some info (similar to UCI/Winboard style)
        time_t current_time = time(NULL);
        int elapsed = (int)(current_time - start_time);

        // Convert move to string like "e2e4", with optional promotion symbol
        char move_str[6];
        move_str[0] = 'a' + FILE(result.best_move.from_square);
        move_str[1] = '1' + RANK(result.best_move.from_square);
        move_str[2] = 'a' + FILE(result.best_move.to_square);
        move_str[3] = '1' + RANK(result.best_move.to_square);
        move_str[4] = (result.best_move.promotion_type != NO_TYPE)
                        ? "nbrq"[result.best_move.promotion_type - 1]
                        : '\0';
        move_str[5] = '\0';

        printf("info depth %d score cp %d time %d pv %s\n",
               depth, result.score, elapsed, move_str);

        // 5) Check whether to stop searching due to time constraints
        //    This is arbitrary logic—adapt as needed
        if ((depth >= 3 && elapsed >= thinking_time) ||
            elapsed >= thinking_time * 2) {
            break;
        }
    }

    // Increase ply count to reflect we made a move in the game
    ply++;
    return best_result.best_move;
}
