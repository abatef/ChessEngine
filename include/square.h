#ifndef _SQUARE_H_
#define _SQUARE_H_

#include <SFML/System/Vector2.hpp>
#include <memory>

#include "piece.h"

class Square : public std::enable_shared_from_this<Square> {
   public:
    using SquarePtr = std::shared_ptr<Square>;
    static const sf::Vector2f SQUARE_SIZE;

   private:
    int _x, _y;  // position x, y
    Piece::PiecePtr _occupier;
    EPieceColor _square_color;

   public:
    Square() = default;
    ~Square() = default;
    Square(int x, int y, EPieceColor color);
    int getX() const;
    int getY() const;
    EPieceColor getSquareColor() const;
    bool isOccupied() const;
    void setOccupier(Piece::PiecePtr occupier);
    Piece::PiecePtr getOccupier() const;
    void clear();
};

#endif