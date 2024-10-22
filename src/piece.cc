#include "piece.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <algorithm>
#include <memory>

#include "board.h"
#include "square.h"
#include "texture_factory.h"

Piece::Piece(EPieceType type, EPieceColor color)
    : m_Type(type), m_Color(color), m_IsFirstMove(false) {
    m_Sprite.setTexture(TextureFactory::getTexture(color, type));
    TextureFactory::getTexture(color, type).setSmooth(true);
    sf::Vector2u origSize = m_Sprite.getTexture()->getSize();
    float scaleX = 100.f / origSize.x;
    float scaleY = 100.f / origSize.y;
    m_Sprite.setScale(scaleX, scaleY);
}

EPieceColor Piece::getColor() const { return m_Color; }
EPieceType Piece::getType() const { return m_Type; }

// bool Piece::canMoveTo(Square::SquarePtr square) const {
//     int x = square->getX();
//     int y = square->getY();
//     return std::any_of(m_PossibleMoves.begin(), m_PossibleMoves.end(),
//                        [&](const auto& move) { return move.first == x && move.second == y; });
// }

void Piece::deOccupy() { m_Square = nullptr; }

bool Piece::moveTo(Square::SquarePtr square) {
    // if (!canMoveTo(square)) {
    //     throw std::runtime_error("Can't move to (" + std::to_string(square->getX()) + "," +
    //                              std::to_string(square->getY()) + ")");
    // }
    if (square->isOccupied()) {
        square->getOccupier()->deOccupy();
        square->clear();
    }
    square->setOccupier(shared_from_this());
    m_Square = square;
    m_Sprite.setPosition(square->getX(), square->getY());
    return true;
}

void Piece::setSquare(Square::SquarePtr sq) { m_Square = sq; }

sf::Vector2f Piece::getSpritePosition() { return m_Sprite.getPosition(); }
sf::Sprite& Piece::getSprite() { return m_Sprite; }
void Piece::setSpritePosition(sf::Vector2f pos) { m_Sprite.setPosition(pos); }

bool Piece::isFirstMove() const { return m_IsFirstMove; }

void Piece::setFirstMove() { m_IsFirstMove = true; }