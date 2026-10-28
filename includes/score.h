#ifndef SCORE_H
#define SCORE_H

#define DOUBLE_PAWN_BONUS 25
#define ISOLATED_PAWN_BONUS 20
#define PASSED_PAWN_BONUS 40
#define ROOK_OPEN_FILE_BONUS 25
#define ROOK_SEMI_OPEN_FILE_BONUS 10
#define CONTEMPT_FACTOR 50
#define MATERIAL_THRESHOLD 200



static const int MOBILITY_BONUS[6] = {
	0,  // Pawn
	0,  // Knight
	1,  // Bishop
	1,  // Rook
	1,  // Queen
	0   // King
};



#endif
