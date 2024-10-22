#ifndef _BOARD_H_
#define _BOARD_H_

#include <SFML/System/Vector2.hpp>
#include <memory>
#include <vector>

#include "piece.h"
#include "square.h"
class Board : public std::enable_shared_from_this<Board> {
   public:
    using BoardT = std::vector<std::vector<Square::SquarePtr>>;
    using BoardPtr = std::shared_ptr<Board>;
    Board() = default;
    bool init();
    BoardT &getSquares();
    void placePiece(int x, int y, EPieceType, EPieceColor);
    Square::SquarePtr selectSquare(sf::Vector2i);
    Square::SquarePtr squareAt(sf::Vector2i);

   private:
    BoardT m_Squares;
};

#endif