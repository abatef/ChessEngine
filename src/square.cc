#include "square.h"

#include <SFML/System/Vector2.hpp>
#include <iostream>

#include "piece.h"

const sf::Vector2f Square::SQUARE_SIZE{100, 100};

Square::Square(int x, int y, EPieceColor color) : _x(x), _y(y), _square_color(color) {}

int Square::getX() const { return _x; }
int Square::getY() const { return _y; }

bool Square::isOccupied() const {
    // std::cout << "Occupied" << std::endl;
    return static_cast<bool>(_occupier);
}
void Square::setOccupier(Piece::PiecePtr occupier) {
    if (_occupier) {
        _occupier->deOccupy();
    }
    _occupier = occupier;
};

Piece::PiecePtr Square::getOccupier() const { return _occupier; }

void Square::clear() {
    if (_occupier) {
        _occupier->deOccupy();
        _occupier = nullptr;
    }
}

EPieceColor Square::getSquareColor() const { return _square_color; }