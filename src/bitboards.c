#include "types.h"
#include "position.h"
#include "move.h"
#include "bitboards.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

Magic bishop_magics[64];
Magic rook_magics[64];

static Bitboard bishop_masks[64];
static Bitboard rook_masks[64];

static Bitboard bishop_attacks[64][512];
static Bitboard rook_attacks [64][4096];

static const int bishop_directions[4][2] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}};
static const int rook_directions[4][2] = {{-1,0}, {0,-1}, {1,0}, {0,1}};

//helper functions magic bitboard
static Bitboard generate_bishop_mask(int square);
static Bitboard generate_rook_mask(int square);
static Bitboard generate_bishop_attacks(int square, Bitboard blockers);
static Bitboard generate_rook_attacks(int square, Bitboard blockers);
static int count_bits(Bitboard b);
static int transform_index(Bitboard blockers, Bitboard magic, int bits);

static bool is_en_passant(const struct position *pos, struct move move);
static bool is_castling(const struct position *pos, struct move move);

void	init_magic_bitboards(void)
{
	for (int square = 0; square < 64; square++)
	{
		bishop_masks[square] = generate_bishop_mask(square);
		int bishop_bits = count_bits(bishop_masks[square]);
		bishop_magics[square].mask = bishop_masks[square];
        bishop_magics[square].magic = bishop_magic_numbers[square];
        bishop_magics[square].shift = 64 - bishop_bits;
        bishop_magics[square].attacks = bishop_attacks[square];

		Bitboard blockers = 0;
		do
		{
			Bitboard attack = generate_bishop_attacks(square, blockers);
			int index = transform_index(blockers, bishop_magic_numbers[square], bishop_bits);
			bishop_attacks[square][index] = attack;

			blockers = (blockers - bishop_masks[square]) & bishop_masks[square];
		} while (blockers);
		rook_masks[square] = generate_rook_mask(square);
		int rook_bits = count_bits(rook_masks[square]);
		rook_magics[square].mask = rook_masks[square];
        rook_magics[square].magic = rook_magic_numbers[square];
        rook_magics[square].shift = 64 - rook_bits;
        rook_magics[square].attacks = rook_attacks[square];

		blockers = 0;
		do
		{
			Bitboard attack = generate_rook_attacks(square, blockers);
			int index = transform_index(blockers, rook_magic_numbers[square], rook_bits);
			rook_attacks[square][index] = attack;

			blockers = (blockers - rook_masks[square]) & rook_masks[square];
		} while (blockers);
	}
	printf("Magic bitboards initialized\n");
}

Bitboard get_bishop_attacks(int square, Bitboard occu)
{
	Magic* magic = &bishop_magics[square];
	occu &= magic->mask;
	int index = transform_index(occu, magic->magic, 64 - magic->shift);
	return magic->attacks[index];
}

Bitboard get_rook_attacks(int square, Bitboard occu)
{
	Magic* magic = &rook_magics[square];
	occu &= magic->mask;
	int index = transform_index(occu, magic->magic, 64 - magic->shift);
	return magic->attacks[index];
}

Bitboard get_queen_attacks(int square, Bitboard occu)
{
	return get_bishop_attacks(square, occu) | get_rook_attacks(square, occu);
}

static int transform_index(Bitboard blockers, Bitboard magic, int bits)
{
    return (int)(((blockers * magic) >> (64 - bits)) & ((1ULL << bits) -1));
}

static int count_bits(Bitboard b)
{
    return __builtin_popcountll(b);
}

static Bitboard generate_bishop_mask(int square)
{
    Bitboard mask = 0;
    int rank = square / 8;
    int file = square % 8;

    for (int i = 0; i < 4; i++)
	{
        int dr = bishop_directions[i][0];
        int df = bishop_directions[i][1];

        for (int j = 1; j < 7; j++)
		{
            int r = rank + j * dr;
            int f = file + j * df;

            if (r < 0 || r > 7 || f < 0 || f > 7)
				break;
            int target_square = r * 8 + f;
            mask |= (1ULL << target_square);
            if (r == 0 || r == 7 || f == 0 || f == 7)
				 break;
        }
    }
    return mask;
}

static Bitboard generate_rook_mask(int square) {
    Bitboard mask = 0;
    int rank = square / 8;
    int file = square % 8;

    for (int i = 0; i < 4; i++)
	{
        int dr = rook_directions[i][0];
        int df = rook_directions[i][1];

        for (int j = 1; j < 7; j++)
		{
            int r = rank + j * dr;
            int f = file + j * df;

            if (r < 0 || r > 7 || f < 0 || f > 7)
				break;
            int target_square = r * 8 + f;
            mask |= (1ULL << target_square);
            if (r == 0 || r == 7 || f == 0 || f == 7)
				break;
        }
    }

    return mask;
}

static Bitboard generate_bishop_attacks(int square, Bitboard blockers) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    for (int i = 0; i < 4; i++)
	{
        int dr = bishop_directions[i][0];
        int df = bishop_directions[i][1];

        for (int j = 1; j <= 7; j++)
		{
            int r = rank + j * dr;
            int f = file + j * df;

            if (r < 0 || r > 7 || f < 0 || f > 7)
				break;
            int target_square = r * 8 + f;
            attacks |= (1ULL << target_square);
            if (blockers & (1ULL << target_square))
				 break;
        }
    }
    return attacks;
}

static Bitboard generate_rook_attacks(int square, Bitboard blockers) {
    Bitboard attacks = 0;
    int rank = square / 8;
    int file = square % 8;

    for (int i = 0; i < 4; i++)
	{
        int dr = rook_directions[i][0];
        int df = rook_directions[i][1];

        for (int j = 1; j <= 7; j++)
		{
            int r = rank + j * dr;
            int f = file + j * df;

            if (r < 0 || r > 7 || f < 0 || f > 7)
				break;
            int target_square = r * 8 + f;
            attacks |= (1ULL << target_square);
            if (blockers & (1ULL << target_square))
				break;
        }
    }

    return attacks;
}

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
	init_magic_bitboards();
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
