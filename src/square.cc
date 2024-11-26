#include "square.h"

#include <SFML/System/Vector2.hpp>
#include <memory>

#include "piece.h"

const sf::Vector2f Square::SQUARE_SIZE{100, 100};

Square::Square(int pX, int pY, EPieceColor pColor)
    : mX(pX)
    , mY(pY)
    , mIsSelected(false)
    , mSquareColor(pColor)
    , mIsHighlighted(false) {}

int Square::getX() const { return mX; }
int Square::getY() const { return mY; }

bool Square::isOccupied() const {
    // std::cout << "Occupied" << std::endl;
    return static_cast<bool>(mOccupier);
}
void Square::setOccupier(Piece::PiecePtr pOccupier) {
    if (mOccupier) {
        mOccupier->deOccupy();
    }
    mOccupier = pOccupier;
};

Piece::PiecePtr Square::getOccupier() const { return mOccupier; }

void Square::clear() {
    if (mOccupier) {
        mOccupier->deOccupy();
        mOccupier = nullptr;
    }
    deSelect();
}

EPieceColor Square::getSquareColor() const { return mSquareColor; }

bool Square::isSelected() const { return mIsSelected; }

void Square::select() { mIsSelected = true; }

void Square::deSelect() { mIsSelected = false; }

sf::Vector2f Square::getPostion() const { return sf::Vector2f(float(mX), float(mY)); }

bool Square::isHighlighted() const { return mIsHighlighted; }

void Square::highlight() { mIsHighlighted = true; }

void Square::clearHighlight() { mIsHighlighted = false; }

void Square::setBoard(std::shared_ptr<Board> pBoard) { mBoard = pBoard; }