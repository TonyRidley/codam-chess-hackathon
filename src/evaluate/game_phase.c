#include "position.h"
#include "types.h"

int get_game_phase(const struct position *pos)
{
	int phase = 24;  // Starting phase value

	for (int square = 0; square < 64; square++)
	{
		int piece = pos->board[square];
		if (piece != NO_PIECE)
		{
			switch (TYPE(piece))
			{
				case KNIGHT:
				case BISHOP:
					phase -= 1;
				break;
				case ROOK:
					phase -= 2;
				break;
				case QUEEN:
					phase -= 4;
				break;
			}
		}
	}
	return phase;
}
