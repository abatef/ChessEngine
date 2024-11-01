#ifndef _PIECE_TEXTURE_H_
#define _PIECE_TEXTURE_H_

#include <SFML/Graphics/Texture.hpp>
#include <iostream>
#include <string>

#include "piece.h"

class TextureFactory {
   public:
    // Static texture variables for each chess piece
    static sf::Texture mWhitePawn;
    static sf::Texture mWhiteRook;
    static sf::Texture mWhiteKnight;
    static sf::Texture mWhiteBishop;
    static sf::Texture mWhiteQueen;
    static sf::Texture mWhiteKing;
    static sf::Texture mBlackPawn;
    static sf::Texture mBlackRook;
    static sf::Texture mBlackKnight;
    static sf::Texture mBlackBishop;
    static sf::Texture mBlackQueen;
    static sf::Texture mBlackKing;

    static sf::Texture LoadTexture(const std::string &pPath) {
        sf::Texture texture;
        if (!texture.loadFromFile(pPath)) {
            std::cerr << "Failed to load texture from: " << pPath << std::endl;
        }
        return texture;
    }

    static sf::Texture &getTexture(EPieceColor pColor, EPieceType pType);
};

#endif  // _PIECE_TEXTURE_H_
