/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   search.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: dsewlia <dsewlia@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 10:01:13 by dsewlia           #+#    #+#             */
/*   Updated: 2025/03/13 12:04:34 by dsewlia          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "search.h"
#include "evaluate.h"
#include "generate.h"
#include "types.h"
#include "zobrist_keys.h"
#include <limits.h>
#include <stdbool.h>

t_table	table[TABLE_MAX];

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
			int color_offset = color * 6;   // 0 for white, 6 for black
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


void	store_results(__uint64_t hash, int score, int depth, struct move best_move)
{
	int	index = hash % TABLE_MAX;
	table[index].hash = hash;
	table[index].score = score;
	table[index].depth = depth;
	table[index].best_move = best_move;
}

bool	get_results(__uint64_t hash, int *prev_score, int depth, struct move *prev_move)
{
	int	index = hash % TABLE_MAX;
	if (table[index].hash == hash && table[index].depth >= depth)
	{
		*prev_score = table[index].score;
		*prev_move = table[index].best_move;
		return (true);
	}
	return (false);
}

#include "search.h"
#include "evaluate.h"
#include "generate.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
// History scoring for move ordering
static int history[2][64][64]; // [side][from][to]
// Simple move scoring function
static int score_move(const struct position *pos, struct move move) {
    int score = 0;
    int piece = pos->board[move.from_square];
    int captured = pos->board[move.to_square];
    // Captures
    if (captured != NO_PIECE) {
        // Score captures by MVV-LVA (Most Valuable Victim - Least Valuable Aggressor)
        // Capturing higher value pieces with lower value pieces gets higher score
        score = 10 * piece_value[TYPE(captured)] - piece_value[TYPE(piece)];
    }
    // Promotions
    if (move.promotion_type != NO_TYPE) {
        score += piece_value[move.promotion_type];
    }
    // Add history score
    score += history[pos->side_to_move][move.from_square][move.to_square];
    return score;
}
// Simple bubble sort for move ordering
static void order_moves(const struct position *pos, struct move *moves, size_t count) {
    for (size_t i = 0; i < count - 1; i++) {
        for (size_t j = 0; j < count - i - 1; j++) {
            if (score_move(pos, moves[j]) < score_move(pos, moves[j+1])) {
                // Swap moves
                struct move temp = moves[j];
                moves[j] = moves[j+1];
                moves[j+1] = temp;
            }
        }
    }
}
// Improved minimax with alpha-beta pruning and move ordering
struct search_result minimax(const struct position *pos, int depth, int alpha, int beta) {
    struct search_result result;
    result.score = -1000000;
    // Base case: Leaf node, evaluate position
    if (depth == 0) {
        result.score = evaluate(pos);
        return result;
    }
    // Generate legal moves and sort them
    struct move moves[MAX_MOVES];
    size_t count = generate_legal_moves(pos, moves);
    // Check for checkmate or stalemate
    if (count == 0) {
        result.score = -900000; // A very bad score, but not the worst
        return result;
    }
    // Order moves based on scoring heuristic
    order_moves(pos, moves, count);
    // Try each move
    for (size_t i = 0; i < count; i++) {
        struct position copy = *pos;
        do_move(&copy, moves[i]);
        // Recursive minimax call with negamax framework
        int score = -minimax(&copy, depth - 1, -beta, -alpha).score;
        // Update best move if we found a better one
        if (score > result.score) {
            result.move = moves[i];
            result.score = score;
            // Update alpha if we improved our position
            if (score > alpha) {
                alpha = score;
                // Update history for good moves that aren't captures
                if (pos->board[moves[i].to_square] == NO_PIECE) {
                    history[pos->side_to_move][moves[i].from_square][moves[i].to_square] += depth * depth;
                }
            }
            // Beta cutoff - opponent won't allow this position
            if (alpha >= beta) {
                break;
            }
        }
    }
    return result;
}
// Main search function with iterative deepening
struct move search(struct search_info *info) {
	static int ply = 0;
	if (info->book && ply < 20)
	{
		printf("test\n");
		struct move book_move = get_book_move(info->pos, info->book, ply);
		printf("From square: %d\n", book_move.from_square);
		printf("To square: %d\n", book_move.to_square);
		if (book_move.from_square && book_move.to_square)
		{
			ply++;
			return book_move;
		}
	}

	// Initialize best_result BEFORE using it
	struct search_result best_result = {0}; 
    // Reset history table before searching
    memset(history, 0, sizeof(history));
    int max_depth = 5; // Moderate depth
    int thinking_time = 1; // 1 second thinking time
    time_t start_time = time(NULL);
    time_t current_time;
    printf("Starting search...\n");
    // Iterative deepening loop
    for (int depth = 1; depth <= max_depth; depth++) {
        // Search at current depth
        struct search_result result = minimax(info->pos, depth, -1000000, 1000000);
        // Save this result
        best_result = result;
        // Log current depth results
        current_time = time(NULL);
        int elapsed = (int)(current_time - start_time);
        // Print move in algebraic notation
        char move_str[6];
        move_str[0] = 'a' + FILE(result.move.from_square);
        move_str[1] = '1' + RANK(result.move.from_square);
        move_str[2] = 'a' + FILE(result.move.to_square);
        move_str[3] = '1' + RANK(result.move.to_square);
        if (result.move.promotion_type != NO_TYPE) {
            move_str[4] = "nbrq"[result.move.promotion_type - 1];
            move_str[5] = '\0';
        } else {
            move_str[4] = '\0';
        }
        printf("info depth %d score cp %d time %d pv %s\n",
               depth, result.score, elapsed, move_str);
        // Stop if minimum required depth and time has passed
        if (depth >= 3 && elapsed >= thinking_time) {
            break;
        }
        // Also stop if we're taking too long
        if (elapsed >= thinking_time * 2) {
            break;
        }
    }
	ply++;
    return best_result.move;
}
