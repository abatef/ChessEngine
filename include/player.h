#ifndef _PLAYER_H_
#define _PLAYER_H_

#include "piece.h"

struct Player {
    Player() = default;
    Player(EPieceColor color) : m_PlayerColor(color) {}
    EPieceColor m_PlayerColor;
    Player *next;
};

#endif