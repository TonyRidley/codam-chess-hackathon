#include "types.h"
#include "position.h"
#include "move.h"
#include "bitboards.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static bool is_en_passant(const struct position *pos, struct move move);
static bool is_castling(const struct position *pos, struct move move);

void init_bitboards(struct position *pos)
{
	memset(pos->bitboards, 0, sizeof(pos->bitboards));

	for (int square = 0; square < 64; square++)
	{
		int piece = pos->board[square];
		if (piece != NO_PIECE)
		{
			SET_BIT(pos->bitboards[COLOR(piece)][TYPE(piece)], square);
		}
	}
	printf("Bitboards initialized\n");
}

void update_bitboards_with_move(struct move move, struct position *pos)
{
	int piece_to_move = pos->board[move.from_square];
	int color_piece_to_move = COLOR(piece_to_move);
	int type_piece_to_move = TYPE(piece_to_move);
	int piece_to_capture = pos->board[move.to_square];
	int color_piece_to_capture = COLOR(piece_to_capture);
	int type_piece_to_capture = TYPE(piece_to_capture);

	// Toggle the moving piece's bits: remove from 'from_square', add at 'to_square'
	pos->bitboards[color_piece_to_move][type_piece_to_move] ^= (1ULL << move.from_square) | (1ULL << move.to_square);

	// Handle captures
	if (piece_to_capture != NO_PIECE)
	{
		pos->bitboards[color_piece_to_capture][type_piece_to_capture] ^= (1ULL << move.to_square);
	}

	// Handle promotions
	if (move.promotion_type != NO_PROMOTION)
	{
		// Remove the original piece from the 'to_square' bitboard,
		// then add the promoted piece at 'to_square'
		pos->bitboards[color_piece_to_move][type_piece_to_move] ^= (1ULL << move.to_square);
		int promoted_type = move.promotion_type;
		pos->bitboards[color_piece_to_move][promoted_type] |= (1ULL << move.to_square);
	}

	// Handle en passant
	if (is_en_passant(pos, move))
	{
		int capture_square = (color_piece_to_move == WHITE ? move.to_square - 8 : move.to_square + 8);
		pos->bitboards[color_piece_to_capture][type_piece_to_capture] ^= (1ULL << capture_square);
		pos->board[capture_square] = NO_PIECE;
		printf("En passant\n");
	}

	// Handle castling
	if (is_castling(pos, move))
	{
		// Determine if it’s kingside or queenside by comparing to/from file
		int fromFile = move.from_square % 8;
		int toFile   = move.to_square % 8;
		int rank     = move.from_square / 8;
		bool kingside = (toFile > fromFile);

		// Decide the rook’s source and destination squares
		int rookFromSquare = kingside ? (rank * 8 + 7) : (rank * 8 + 0);
		int rookToSquare   = kingside ? (rank * 8 + 5) : (rank * 8 + 3);

		// Update the board array for the rook
		int rookPiece = pos->board[rookFromSquare];
		pos->board[rookFromSquare] = NO_PIECE;
		pos->board[rookToSquare] = rookPiece;

		// Toggle the rook’s bits in the bitboard
		pos->bitboards[color_piece_to_move][ROOK] ^= (1ULL << rookFromSquare) | (1ULL << rookToSquare);
	}
}

static bool is_en_passant(const struct position *pos, struct move move)
{
	int piece = pos->board[move.from_square];
	if (TYPE(piece) != PAWN)
	{
		return false; // must be a pawn
	}

	// if the 'to_square' square is the en passant square
	if (move.to_square == pos->en_passant_square)
	{
		// and it's a diagonal move
		int fileDistance = abs((move.to_square % 8) - (move.from_square % 8));
		int rankDistance = abs((move.to_square / 8) - (move.from_square / 8));
		if (fileDistance == 1 && rankDistance == 1)
		{
			return true;
		}
	}
	return false;
}

static bool is_castling(const struct position *pos, struct move move)
{
	int piece = pos->board[move.from_square];
	int pieceType = TYPE(piece);

	// Must be the king
	if (pieceType != KING)
	{
		return false;
	}

	// if the 'to_square' square is 2 away from the king’s start
	int fromFile = move.from_square % 8;
	int toFile   = move.to_square % 8;

	if (abs(toFile - fromFile) == 2)
		return true;
	return false;
}

void verify_board_state(const struct position *pos)
{
	for (int square = 0; square < 64; square++) {
		int piece = pos->board[square];
		if (piece != NO_PIECE) {
			int color = COLOR(piece);
			int type = TYPE(piece);

			// Check if the piece is represented in bitboards
			if (!(pos->bitboards[color][type] & (1ULL << square))) {
				printf("Error: Piece on board not in bitboard at square %d\n", square);
			}
		} else {
			// Check no pieces are in bitboards for empty square
			for (int c = 0; c < 2; c++) {
				for (int t = 0; t < 6; t++) {
					if (pos->bitboards[c][t] & (1ULL << square)) {
						printf("Error: Empty square has piece in bitboard at square %d\n", square);
					}
				}
			}
		}
	}
}
