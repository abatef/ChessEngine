#include "square.h"

#include <SFML/System/Vector2.hpp>
#include <iostream>

#include "piece.h"

const sf::Vector2f Square::SQUARE_SIZE{100, 100};

Square::Square(int x, int y, EPieceColor color)
    : m_X(x), m_Y(y), m_IsSelected(false), m_SquareColor(color) {}

int Square::getX() const { return m_X; }
int Square::getY() const { return m_Y; }

bool Square::isOccupied() const {
    // std::cout << "Occupied" << std::endl;
    return static_cast<bool>(m_Occupier);
}
void Square::setOccupier(Piece::PiecePtr occupier) {
    if (m_Occupier) {
        m_Occupier->deOccupy();
    }
    m_Occupier = occupier;
};

Piece::PiecePtr Square::getOccupier() const { return m_Occupier; }

void Square::clear() {
    if (m_Occupier) {
        m_Occupier->deOccupy();
        m_Occupier = nullptr;
    }
}

EPieceColor Square::getSquareColor() const { return m_SquareColor; }

bool Square::isSelected() const { return m_IsSelected; }

void Square::select() { m_IsSelected = true; }

void Square::deSelect() { m_IsSelected = false; }