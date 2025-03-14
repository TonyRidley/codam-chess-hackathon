#include "bitboards.h"
#include "position.h"
#include "types.h"
#include "score.h"

static Bitboard get_piece_attacks(int pieceType, int square, Bitboard occupancy)
{
    printf("Inside get_piece_attacks\n");
    switch (pieceType)
    {
        case BISHOP:
            return get_bishop_attacks(square, occupancy);
        case ROOK:
            return get_rook_attacks(square, occupancy);
        case QUEEN:
            return get_queen_attacks(square, occupancy);
        default:
            return 0ULL;
    }
}

int evaluate_mobility(const struct position *pos)
{
    if (!pos) return 0;

    int score = 0;

    // Just evaluate bishop, rook, and queen mobility for now
    for (int type = BISHOP; type <= QUEEN; type++)
    {
        // Get all pieces occupancy first
        Bitboard occupancy = 0ULL;
        for (int c = 0; c < 2; c++)
            for (int t = 0; t < 6; t++)
                occupancy |= pos->bitboards[c][t];

        // White pieces mobility
        Bitboard pieces = pos->bitboards[WHITE][type];
        while (pieces)
        {
            int square = __builtin_ctzll(pieces);
            if (square >= 0 && square < 64)  // Bounds check
            {
                Bitboard attacks = get_piece_attacks(type, square, occupancy);
                Bitboard white_occupied = 0ULL;
                for (int t = 0; t < 6; t++)
                    white_occupied |= pos->bitboards[WHITE][t];

                // Only count squares not occupied by own pieces
                Bitboard mobility = attacks & ~white_occupied;
                score += MOBILITY_BONUS[type] * __builtin_popcountll(mobility);
            }
            pieces &= pieces - 1;  // Clear LSB
        }

        // Black pieces mobility
        pieces = pos->bitboards[BLACK][type];
        while (pieces)
        {
            int square = __builtin_ctzll(pieces);
            if (square >= 0 && square < 64)  // Bounds check
            {
                Bitboard attacks = get_piece_attacks(type, square, occupancy);
                Bitboard black_occupied = 0ULL;
                for (int t = 0; t < 6; t++)
                    black_occupied |= pos->bitboards[BLACK][t];

                // Only count squares not occupied by own pieces
                Bitboard mobility = attacks & ~black_occupied;
                score -= MOBILITY_BONUS[type] * __builtin_popcountll(mobility);
            }
            pieces &= pieces - 1;  // Clear LSB
        }
    }

    return score;
}
