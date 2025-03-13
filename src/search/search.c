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
#include <limits.h>
#include <stdbool.h>

t_table	table[TABLE_MAX];



// void	init_z_hash()
// {
// 	for (int i = 0; i < 64; i++)
// 	{
// 		for (int j = 0; j < 12; j++)
// 		{
// 			z_table[i][j] = get_random64();
// 		}
// 	}
// }

__uint64_t	get_hash(const struct position *pos)
{
	__uint64_t	hash = 0;
	for (int i = 0; i < 64; i++)
	{
		if (pos->board[i] != NO_PIECE)
		{
			hash ^= z_table[i][pos->board[i]];
		}
	}
	return (hash);
}

__uint64_t update_z_table(__uint64_t hash, int from, int to, int piece)
{
	hash ^= z_table[from][piece];
	hash ^= z_table[to][piece];
	return (hash);
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

struct search_result minimax(const struct position *pos, int depth, int alpha, int beta)
{
	struct search_result result;

	__uint64_t	hash = get_hash(pos);
	struct move	prev_move;
	int			prev_score;
	if (get_results(hash, &prev_score, depth, &prev_move) == true)
	{
		result.score = prev_score;
		result.move = prev_move;
		return (result);
	}
	if (depth == 0) {
		/* we have reached our search depth, so evaluate the position.       */
		result.score = evaluate(pos);
		return (result);
	}
	struct move moves[MAX_MOVES];
	size_t count = generate_legal_moves(pos, moves);
	bool maxi = (depth % 2 == 0);
	result.score = (maxi == true) ? INT_MIN : INT_MAX;

	for (size_t i = 0; i < count; i++)
	{
		struct position copy = *pos;
		int score;
		int	piece = pos->board[moves[i].from_square];
		/* do a move, the current player in `copy` is then the opponent, */
		/* and so when we call minimax we get the score of the opponent. */
		do_move(&copy, moves[i]);
		uint64_t new_hash = update_z_table(hash, moves[i].from_square, moves[i].to_square, piece); 
		/* minimax is called recursively. this call returns the score of */
		/* the opponent, so we must negate it to get our score.          */
		score = minimax(&copy, depth - 1, alpha, beta).score;
		/* update the best move if we found a better one.                */
		if (maxi)
		{
			if (score > result.score)
			{
				result.score = score;
				result.move = moves[i];
			}
			alpha = (score > alpha) ? score : alpha;
		}
		else
		{
			if (score < result.score)
			{
				result.score = score;
				result.move = moves[i];
			}
			beta = (score < beta) ? score : beta;
		}
		if (alpha >= beta)
			break ;
	}
	store_results(hash, result.score, depth, result.move);
	return result;
}

struct move search(const struct search_info *info)
{
	return minimax(info->pos, 6, INT_MIN, INT_MAX).move;
}
