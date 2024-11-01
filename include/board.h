#ifndef _BOARD_H_
#define _BOARD_H_

#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

#include "piece.h"
#include "square.h"

class Board : public std::enable_shared_from_this<Board> {
   public:
    using BoardSquares = std::vector<std::vector<Square::SquarePtr>>;
    using BoardPtr = std::shared_ptr<Board>;
    using BoardPieces = std::vector<Piece::PiecePtr>;

   public:
    Board() = default;
    bool init();
    BoardSquares &getSquares();
    BoardPieces &getPieces();
    Square::SquarePtr selectSquare(sf::Vector2i pSquarePosition);
    Square::SquarePtr squareAt(sf::Vector2i pSquarePosition);

   private:
    void placePiece(int pX, int pY, EPieceType pType, EPieceColor pColor);

   private:
    BoardSquares mSquares;
    BoardPieces mPieces;
};

#endif