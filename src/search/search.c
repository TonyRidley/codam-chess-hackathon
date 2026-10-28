#include "search.h"
#include "evaluate.h"
#include "generate.h"
#include "types.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

static int history[2][64][64];

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

    if (depth == 0) {
        result.score = evaluate(pos);
        return result;
    }

    struct move moves[MAX_MOVES];
    size_t count = generate_legal_moves(pos, moves);

    if (count == 0) {
        result.score = -900000;
        return result;
    }

    order_moves(pos, moves, count);

    for (size_t i = 0; i < count; i++) {
        struct position copy = *pos;
        do_move(&copy, moves[i]);
        int score = -minimax(&copy, depth - 1, -beta, -alpha).score;

        if (score > result.score) {
            result.move = moves[i];
            result.score = score;
            if (score > alpha) {
                alpha = score;
                if (pos->board[moves[i].to_square] == NO_PIECE) {
                    history[pos->side_to_move][moves[i].from_square][moves[i].to_square] += depth * depth;
                }
            }
            if (alpha >= beta) {
                break;
            }
        }
    }
    return result;
}

struct move search(struct search_info *info) {
    static int ply = 0;

    if (info->book && ply < 20) {
        struct move book_move = get_book_move(info->pos, info->book, ply);
        if (book_move.from_square && book_move.to_square) {
            ply++;
            return book_move;
        }
    }

    struct search_result best_result = {0};
    memset(history, 0, sizeof(history));

    const int max_depth = 5;
    const int thinking_time = 1;
    time_t start_time = time(NULL);

    for (int depth = 1; depth <= max_depth; depth++) {
        struct search_result result = minimax(info->pos, depth, -1000000, 1000000);
        best_result = result;

        time_t current_time = time(NULL);
        int elapsed = (int)(current_time - start_time);

        char move_str[6];
        move_str[0] = 'a' + FILE(result.move.from_square);
        move_str[1] = '1' + RANK(result.move.from_square);
        move_str[2] = 'a' + FILE(result.move.to_square);
        move_str[3] = '1' + RANK(result.move.to_square);
        move_str[4] = (result.move.promotion_type != NO_TYPE) ?
                      "nbrq"[result.move.promotion_type - 1] : '\0';
        move_str[5] = '\0';

        printf("info depth %d score cp %d time %d pv %s\n",
               depth, result.score, elapsed, move_str);

        if ((depth >= 3 && elapsed >= thinking_time) ||
            elapsed >= thinking_time * 2) {
            break;
        }
    }

    ply++;
    return best_result.move;
}
