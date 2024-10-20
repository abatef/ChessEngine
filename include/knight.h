#ifndef _KNIGHT_H_
#define _KNIGHT_H_

#include "piece.h"
class Knight : public Piece {
   public:
    Knight(EPieceColor);
    void generateMoves() override;
};

#endif
