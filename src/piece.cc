#include "piece.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>

#include "piece_texture.h"
#include "square.h"

Piece::Piece(EPieceType type, EPieceColor color) : _type(type), _color(color) {
    _sprite.setTexture(PieceTexture::getTexture(color, type));
    PieceTexture::getTexture(color, type).setSmooth(true);
    sf::Vector2u origSize = _sprite.getTexture()->getSize();
    float scaleX = 100.f / origSize.x;
    float scaleY = 100.f / origSize.y;
    _sprite.setScale(scaleX, scaleY);
}

EPieceColor Piece::getColor() const { return _color; }
EPieceType Piece::getType() const { return _type; }

bool Piece::canMoveTo(Square::SquarePtr square) const {
    int x = square->getX();
    int y = square->getY();
    return std::any_of(_moves.begin(), _moves.end(),
                       [&](const auto& move) { return move.first == x && move.second == y; });
}

void Piece::deOccupy() { _square = nullptr; }

bool Piece::moveTo(Square::SquarePtr square) {
    if (!canMoveTo(square)) {
        throw std::runtime_error("Can't move to (" + std::to_string(square->getX()) + "," +
                                 std::to_string(square->getY()) + ")");
    }
    if (square->isOccupied()) {
        square->getOccupier()->deOccupy();
        square->clear();
    }
    square->setOccupier(shared_from_this());
    _square = square;
    _sprite.setPosition(square->getX(), square->getY());
    return true;
}

void Piece::setSquare(Square::SquarePtr sq) { _square = sq; }

sf::Vector2f Piece::getSpritePosition() { return _sprite.getPosition(); }
sf::Sprite& Piece::getSprite() { return _sprite; }
void Piece::setSpritePosition(sf::Vector2f pos) { _sprite.setPosition(pos); }