#include "evaluate.h"
#include "types.h"

const int piece_value[6] = { 100, 320, 330, 500, 900, 20000 };

static inline int is_checkmate(const struct position *pos)
{
	int king_square;
	Bitboard king_bb = pos->bitboards[pos->side_to_move][KING];
	king_square = __builtin_ctzll(king_bb);
	
	if (!is_square_attacked(pos, king_square, !pos->side_to_move))
		return 0;

	struct move moves[MAX_MOVES];
	size_t count = generate_legal_moves(pos, moves);

	return (count == 0);
}

static inline int evaluate_material(const struct position *pos)
{
	int score[2] = { 0, 0 };

	for (int color = WHITE; color <= BLACK; color++) {
		for (int piece_type = PAWN; piece_type <= KING; piece_type++) {
			Bitboard piece_bb = pos->bitboards[color][piece_type];
			int piece_count = __builtin_popcountll(piece_bb);
			score[color] += piece_value[piece_type] * piece_count;
		}
	}

	return score[WHITE] - score[BLACK];
}

// Calculate piece square table values using bitboards
static inline int evaluate_piece_square_tables(const struct position *pos, float phase_pct)
{
	int score[2] = { 0, 0 };

	for (int color = WHITE; color <= BLACK; color++) {
		for (int piece_type = PAWN; piece_type <= KING; piece_type++) {
			Bitboard piece_bb = pos->bitboards[color][piece_type];
			while (piece_bb) {
				int square = __builtin_ctzll(piece_bb);
				int piece = pos->board[square];
				score[color] += get_piece_square_value(piece, square, phase_pct);
				piece_bb &= (piece_bb - 1);
			}
		}
	}

	return score[WHITE] - score[BLACK];
}

int evaluate(const struct position *pos)
{
	if(is_checkmate(pos))
		return -30000;
	float phase_pct = get_game_phase(pos);

	int material_score = evaluate_material(pos);
	int pst_score = evaluate_piece_square_tables(pos, phase_pct);
	int positional_score = evaluate_pawn_structure(pos) + evaluate_rook(pos);
	int end_game_score = evaluate_king_corner_endgame(pos, phase_pct);

	int final_score = material_score + pst_score + positional_score + end_game_score;
	return (pos->side_to_move == WHITE) ? final_score : -final_score;
}
