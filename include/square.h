#ifndef _SQUARE_H_
#define _SQUARE_H_

#include <SFML/System/Vector2.hpp>
#include <memory>

#include "piece.h"

class Board;

class Square : public std::enable_shared_from_this<Square> {
   public:
    using SquarePtr = std::shared_ptr<Square>;
    static const sf::Vector2f SQUARE_SIZE;
    std::shared_ptr<Board> mBoard;

   private:
    int mX, mY;  // position x, y
    bool mIsSelected;
    bool mIsHighlighted;
    Piece::PiecePtr mOccupier;
    EPieceColor mSquareColor;

   public:
    Square() = default;
    ~Square() = default;
    Square(int pX, int pY, EPieceColor pColor);
    int getX() const;
    int getY() const;
    EPieceColor getSquareColor() const;
    bool isOccupied() const;
    bool isSelected() const;
    bool isHighlighted() const;
    void setOccupier(Piece::PiecePtr pOccupier);
    void setBoard(std::shared_ptr<Board> pBoard);
    Piece::PiecePtr getOccupier() const;
    void clear();
    void select();
    void deSelect();
    void highlight();
    void clearHighlight();
    sf::Vector2f getPostion() const;
};

#endif