#include "search.h"
#include "evaluate.h"
#include "generate.h"
#include <limits.h>
#include <stdbool.h>

struct search_result minimax(const struct position *pos, int depth, int alpha, int beta)
{
	struct search_result result;

	result.score = (depth % 2 == 0) ? INT_MIN : INT_MAX;

	if (depth == 0) {
		/* we have reached our search depth, so evaluate the position.       */
		result.score = evaluate(pos);
		return (result);
	}
	struct move moves[MAX_MOVES];
	size_t count = generate_legal_moves(pos, moves);
	bool maxi = (depth % 2 == 0);
	result.score = (maxi == true) ? INT_MIN : INT_MAX;

	for (size_t index = 0; index < count; index++)
	{
		struct position copy = *pos;
		int score;
		/* do a move, the current player in `copy` is then the opponent, */
		/* and so when we call minimax we get the score of the opponent. */
		do_move(&copy, moves[index]);
		/* minimax is called recursively. this call returns the score of */
		/* the opponent, so we must negate it to get our score.          */
		score = minimax(&copy, depth - 1, alpha, beta).score;
		/* update the best move if we found a better one.                */
		if (maxi)
		{
			if (score > result.score)
			{
				result.score = score;
				result.move = moves[index];
			}
			alpha = (score > alpha) ? score : alpha;
		}
		else
		{
			if (score < result.score)
			{
				result.score = score;
				result.move = moves[index];
			}
			beta = (score < beta) ? score : beta;
		}
		if (alpha >= beta)
			break ;
	}
	return result;
}

struct move search(const struct search_info *info)
{
	return minimax(info->pos, 4, INT_MIN, INT_MAX).move;
}
