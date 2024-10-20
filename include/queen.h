#ifndef _QUEEN_H_
#define _QUEEN_H_

#include "piece.h"
class Queen : public Piece {
   public:
    Queen(EPieceColor);
    void generateMoves() override;
};

#endif