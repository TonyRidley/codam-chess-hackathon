/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   search.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 10:01:13 by dsewlia           #+#    #+#             */
/*   Updated: 2025/03/14 03:36:08 by marvin           ###   ########.fr       */
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


	//if leaf node->evaluate position
	if (depth == 0)
	{
		result.score = evaluate(pos);
		return (result);
	}
	//if maximizing position, evaluate alpha and prune at depth 1
	if (depth % 2 == 0)
	{
		int score;
		struct move moves[MAX_MOVES];
		//to generate count of child nodes
		size_t count = generate_legal_moves(pos, moves);
		result.score = INT_MIN;
		for (size_t i = 0; i < count; i++)
		{
			struct position copy = *pos;
			do_move(&copy, moves[i]);
			//recursive call for next depth
			score = minimax(&copy, depth - 1, alpha, beta).score;
			if (score > result.score)
			{
				result.score = score;
				result.move = moves[i];
			}
			if (score > alpha)
				alpha = score;
			//pruning
			if (alpha >= beta)
				break ;
		}
	}
	//if minimizing position, evaluate beta and prune at depth 1
	else
	{
		int score;
		struct move moves[MAX_MOVES];
		size_t count = generate_legal_moves(pos, moves);
		result.score = INT_MAX;
		for (size_t i = 0; i < count; i++)
		{
			struct position copy = *pos;
			do_move(&copy, moves[i]);
			score = minimax(&copy, depth - 1, alpha, beta).score;
			if (score < result.score)
			{
				result.score = score;
				result.move = moves[i];
			}
			if (score < beta)
				beta = score;
			if (alpha >= beta)
				break ;
		}
	}
	return result;
}

struct move search(const struct search_info *info)
{
	return minimax(info->pos, 6, INT_MIN, INT_MAX).move;
}
