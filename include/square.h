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
    int m_X, m_Y;  // position x, y
    bool m_IsSelected;
    Piece::PiecePtr m_Occupier;
    EPieceColor m_SquareColor;

   public:
    Square() = default;
    ~Square() = default;
    Square(int x, int y, EPieceColor color);
    int getX() const;
    int getY() const;
    EPieceColor getSquareColor() const;
    bool isOccupied() const;
    bool isSelected() const;
    void setOccupier(Piece::PiecePtr occupier);
    Piece::PiecePtr getOccupier() const;
    void clear();
    void select();
    void deSelect();
};

#endif