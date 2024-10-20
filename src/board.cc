#include "board.h"

#include <iostream>
#include <memory>
#include <vector>

#include "bishop.h"
#include "pawn.h"
#include "piece.h"
#include "square.h"

bool Board::init() {
    for (int i = 0; i < 8; i++) {
        std::vector<Square::SquarePtr> row;
        EPieceColor color = (i % 2 == 0) ? EPieceColor::WHITE : EPieceColor::BLACK;

        for (int j = 0; j < 8; j++) {
            Square::SquarePtr sqr = std::make_shared<Square>(i, j, color);
            row.push_back(sqr);
            color = (color == EPieceColor::WHITE) ? EPieceColor::BLACK : EPieceColor::WHITE;
        }

        squares.push_back(row);
    }
    // i -> columns
    for (int i = 0; i < 8; i++) {
        Piece::PiecePtr w_Pawn = std::make_shared<Pawn>(EPieceColor::WHITE);
        squares[i][1]->setOccupier(w_Pawn);
        Piece::PiecePtr b_Pawn = std::make_shared<Pawn>(EPieceColor::BLACK);
        squares[i][6]->setOccupier(b_Pawn);
    }

    Piece::PiecePtr w_Bishop = std::make_shared<Bishop>(EPieceColor::WHITE);
    squares[2][0]->setOccupier(w_Bishop);
    w_Bishop = std::make_shared<Bishop>(EPieceColor::WHITE);
    squares[5][0]->setOccupier(w_Bishop);

    Piece::PiecePtr b_Bishop = std::make_shared<Bishop>(EPieceColor::BLACK);
    squares[2][7]->setOccupier(b_Bishop);
    b_Bishop = std::make_shared<Bishop>(EPieceColor::BLACK);
    squares[5][7]->setOccupier(b_Bishop);

    return true;
}

Board::BoardT &Board::getSquares() { return squares; }