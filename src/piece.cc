#include "piece.h"

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <memory>

#include "square.h"
#include "texture_factory.h"

Piece::Piece(EPieceType pType, EPieceColor pColor)
    : mType(pType)
    , mColor(pColor)
    , mMovedBefore(false) {
    mSprite.setTexture(TextureFactory::getTexture(pColor, pType));
    TextureFactory::getTexture(pColor, pType).setSmooth(true);
    sf::Vector2u origSize = mSprite.getTexture()->getSize();
    float scaleX = 100.f / origSize.x;
    float scaleY = 100.f / origSize.y;
    mSprite.setScale(scaleX, scaleY);
}

EPieceColor Piece::getColor() const { return mColor; }
EPieceType Piece::getType() const { return mType; }

void Piece::deOccupy() { mSquare = nullptr; }

void Piece::setSquare(Square::SquarePtr pSquare) { mSquare = pSquare; }

sf::Vector2f Piece::getSpritePosition() { return mSprite.getPosition(); }
sf::Sprite& Piece::getSprite() { return mSprite; }
void Piece::setSpritePosition(sf::Vector2f pos) { mSprite.setPosition(pos); }

bool Piece::movedBefore() const { return mMovedBefore; }

void Piece::setFirstMove() { mMovedBefore = true; }

std::string Piece::getName() const {
    std::string name = "";
    switch (mColor) {
        case EPieceColor::BLACK: name += "BLACK "; break;
        case EPieceColor::WHITE: name += "WHITE "; break;
    }
    switch (mType) {
        case EPieceType::PAWN: name += "PAWN"; break;
        case EPieceType::ROOK: name += "ROOK"; break;
        case EPieceType::BISHOP: name += "BISHOP"; break;
        case EPieceType::QUEEN: name += "QUEEN"; break;
        case EPieceType::KING: name += "KING"; break;
        case EPieceType::KNIGHT: name += "KNIGHT"; break;
    }
    return name;
}