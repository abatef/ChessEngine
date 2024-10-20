#ifndef _PAWN_H_
#define _PAWN_H_

#include "piece.h"
#include "restricted_piece.h"
class Pawn : public RestrictedPiece {
   public:
    Pawn(EPieceColor color) : RestrictedPiece(EPieceType::PAWN, color) {
        // Initialize Pawn-specific attributes if any
    }
    void generateMoves() override;
};

#endif