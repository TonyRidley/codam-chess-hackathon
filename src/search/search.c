#include "search.h"
#include "evaluate.h"
#include "generate.h"

#include <limits.h>

struct search_result minimax(const struct position *pos, int depth, int alpha, int beta)
{
	struct search_result result;

	result.score = -1000000;

	if (depth == 0) {
		/* we have reached our search depth, so evaluate the position.       */
		result.score = evaluate(pos);
	}
	else
	{
		struct move moves[MAX_MOVES];
		size_t count = generate_legal_moves(pos, moves);
		size_t index;

		for (index = 0; index < count; index++)
		{
			struct position copy = *pos;
			int score;

			/* do a move, the current player in `copy` is then the opponent, */
			/* and so when we call minimax we get the score of the opponent. */
			do_move(&copy, moves[index]);

			/* minimax is called recursively. this call returns the score of */
			/* the opponent, so we must negate it to get our score.          */
			score = -minimax(&copy, depth - 1, alpha, beta).score;

			/* update the best move if we found a better one.                */
			if (score > result.score)
			{
				result.move = moves[index];
				result.score = score;
			}
			if (score > alpha)
				alpha = score;
			if (alpha >= beta)
				break ;
		}
	}

	return result;
}

struct move search(const struct search_info *info)
{
	return minimax(info->pos, 4, INT_MIN, INT_MAX).move;
}
