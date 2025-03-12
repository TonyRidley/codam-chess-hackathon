#include "bitboards.h"
#include "position.h"
#include "types.h"
#include "evaluate.h"
#include <math.h>
#include <stdlib.h>

int evaluate_king_corner_endgame(const struct position *pos, float game_phase)
{
	float endgame_weight = 1.0f - game_phase;

	// Get king squares
	int friendly_king_sq = get_lsb(pos->bitboards[pos->side_to_move][KING]);
	int opponent_king_sq = get_lsb(pos->bitboards[!pos->side_to_move][KING]);

	int score = 0;

	// Get coordinates for opponent king
	int opponent_king_file = opponent_king_sq % 8;
	int opponent_king_rank = opponent_king_sq / 8;

	// Calculate opponent king's distance from center
	int opponent_dist_to_center_file = MAX(3 - opponent_king_file, opponent_king_file - 4);
	int opponent_dist_to_center_rank = MAX(3 - opponent_king_rank, opponent_king_rank - 4);
	int opponent_dist_from_center = opponent_dist_to_center_file + opponent_dist_to_center_rank;

	// Reward pushing opponent king away from center
	score += opponent_dist_from_center;

	// Get coordinates for friendly king
	int friendly_king_file = friendly_king_sq % 8;
	int friendly_king_rank = friendly_king_sq / 8;

	// Calculate distance between kings
	int file_distance = abs(friendly_king_file - opponent_king_file);
	int rank_distance = abs(friendly_king_rank - opponent_king_rank);
	int kings_distance = file_distance + rank_distance;

	// Reward keeping our king close to opponent king
	score += 14 - kings_distance;

	// Apply endgame weight and scaling factor
	return (int)(score * 10 * endgame_weight);
}
