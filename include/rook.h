#ifndef _ROOK_H_
#define _ROOK_H_

#include "piece.h"
class Rook : public Piece {
   public:
    Rook(EPieceColor);
    void generateMoves() override;
};

#endif