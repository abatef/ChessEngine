#ifndef _RESTRICTED_PIECE_H_
#define _RESTRICTED_PIECE_H_

#include <SFML/Graphics/Sprite.hpp>

#include "piece.h"

struct RestrictedPiece : public Piece {
    RestrictedPiece(EPieceType type, EPieceColor color) : Piece(type, color) {}
    bool _moved;
    bool hasMoved() const { return _moved; }
};

#endif