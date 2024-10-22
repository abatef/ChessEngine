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
    std::shared_ptr<Board> m_Board;

   private:
    int m_X, m_Y;  // position x, y
    bool m_IsSelected;
    bool m_IsHighlighted;
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
    bool isHighlighted() const;
    void setOccupier(Piece::PiecePtr occupier);
    void setBoard(std::shared_ptr<Board>);
    Piece::PiecePtr getOccupier() const;
    void clear();
    void select();
    void deSelect();
    void highlight();
    void clearHighlight();
    sf::Vector2f getPostion() const;
};

#endif