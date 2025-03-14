/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   search.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: marvin <marvin@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/13 10:01:13 by dsewlia           #+#    #+#             */
/*   Updated: 2025/03/14 03:17:29 by marvin           ###   ########.fr       */
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

void	copy_position(struct position *dest, const struct position *src)
{
	memcpy(dest, src, sizeof(struct position));
}

//iterative minimax function with alpha beta pruning
struct search_result minimax(const struct position *pos, int depth, int alpha, int beta)
{
	struct move moves_d1[MAX_MOVES];
	size_t	count_d1 = generate_legal_moves(pos, moves_d1);
	struct search_result result_d1;
	result_d1.move = moves_d1[0];
	result_d1.score = INT_MIN;
	//depth 1 -> maximizer
	for (size_t i_d1 = 0; i_d1 < count_d1; i_d1++)
	{
		struct position copy_d1;
		copy_position(&copy_d1, pos);
		do_move(&copy_d1, moves_d1[i_d1]);
		struct move moves_d2[MAX_MOVES];
		size_t count_d2 = generate_legal_moves(&copy_d1, moves_d2);
		//depth 2 -> minimizer
		struct search_result result_d2;
		result_d2.score = INT_MAX;
		for (size_t i_d2 = 0; i_d2 < count_d2; i_d2++)
		{
			struct position copy_d2;
			copy_position(&copy_d2, &copy_d1);
			do_move(&copy_d2, moves_d2[i_d2]);
			struct move moves_d3[MAX_MOVES];
			size_t count_d3 = generate_legal_moves(&copy_d2, moves_d3);
			//DEPTH 3 -> maximizer
			struct search_result result_d3;
			result_d3.score = INT_MIN;
			for (size_t i_d3 = 0; i_d3 < count_d3; i_d3++)
			{
				struct position copy_d3;
				copy_position(&copy_d3, &copy_d2);
				do_move(&copy_d3, moves_d3[i_d3]);
				struct move moves_d4[MAX_MOVES];
				size_t	count_d4 = generate_legal_moves(&copy_d3, moves_d4);
				//depth 4 -> minimizer
				struct search_result result_d4;
				result_d4.score = INT_MAX;
				for (size_t i_d4 = 0; i_d4 < count_d4; i_d4++)
				{
					struct position copy_d4;
					copy_position(&copy_d4, &copy_d3);
					do_move(&copy_d4, moves_d4[i_d4]);
					struct move moves_d5[MAX_MOVES];
					size_t count_d5 = generate_legal_moves(&copy_d4, moves_d5);
					//DEPTH 5 -> Maximizer
					struct search_result result_d5;
					result_d5.score = INT_MIN;
					for(size_t i_d5 = 0; i_d5 < count_d5; i_d5++)
					{
						struct position copy_d5;
						copy_position(&copy_d5, &copy_d4);
						do_move(&copy_d5, moves_d5[i_d5]);
						struct move moves_d6[MAX_MOVES];
						size_t count_d6 = generate_legal_moves(&copy_d5, moves_d6);
						//depth 6 -> minimizer.
						struct search_result result_d6;
						result_d6.score = INT_MAX;
						for (size_t i_d6 = 0; i_d6 < count_d6; i_d6++)
						{
							int score;
							struct position copy_d6;
							copy_position(&copy_d6, &copy_d5);
							do_move(&copy_d6, moves_d6[i_d6]);
							score = evaluate(&copy_d6);
							if (score < result_d6.score)
							{
								result_d6.score = score;
								result_d6.move = moves_d6[i_d6];
							}
							if (score < beta)
								beta = score;
							if (alpha >= beta)
								break ;
						}
						if (result_d6.score > result_d5.score)
						{
							result_d5.score = result_d6.score;
							result_d5.move = result_d6.move;
						}
						if (result_d5.score > alpha)
							alpha = result_d5.score;
						if (alpha >= beta)
							break ;
					}
					if (result_d5.score < result_d4.score)
					{
						result_d4.score = result_d5.score;
						result_d4.move = result_d5.move;
					}
					if (result_d4.score < beta)
						beta = result_d4.score;
					if (alpha >= beta)
						break ;
				}
				if (result_d4.score > result_d3.score)
				{
					result_d3.score = result_d4.score;
					result_d3.move = result_d4.move;
				}
				if (result_d3.score > alpha)
					alpha = result_d3.score;
				if (alpha >= beta)
					break ;
			}
			if (result_d3.score < result_d2.score)
			{
				result_d2.score = result_d3.score;
				result_d2.move = result_d3.move;
			}
			if (result_d2.score < beta)
				beta = result_d2.score;
			if (alpha >= beta)
				break ;
		}
		if (result_d2.score > result_d1.score)
		{
			result_d1.score = result_d2.score;
			result_d1.move = result_d2.move;
		}
		if (result_d1.score > alpha)
			alpha = result_d1.score;
		if (alpha >= beta)
			break ;
	}
	return (result_d1);
}

struct move search(const struct search_info *info)
{
	return minimax(info->pos, 6, INT_MIN, INT_MAX).move;
}
