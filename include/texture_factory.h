#ifndef _PIECE_TEXTURE_H_
#define _PIECE_TEXTURE_H_

#include <SFML/Graphics/Texture.hpp>
#include <iostream>
#include <string>

#include "piece.h"

class TextureFactory {
   public:
    // Static texture variables for each chess piece
    static sf::Texture white_pawn_;
    static sf::Texture white_rook_;
    static sf::Texture white_knight_;
    static sf::Texture white_bishop_;
    static sf::Texture white_queen_;
    static sf::Texture white_king_;
    static sf::Texture black_pawn_;
    static sf::Texture black_rook_;
    static sf::Texture black_knight_;
    static sf::Texture black_bishop_;
    static sf::Texture black_queen_;
    static sf::Texture black_king_;

    static sf::Texture LoadTexture(const std::string &path) {
        sf::Texture texture;
        if (!texture.loadFromFile(path)) {
            std::cerr << "Failed to load texture from: " << path << std::endl;
        }
        return texture;
    }

    static sf::Texture &getTexture(EPieceColor color, EPieceType type);
};

#endif  // _PIECE_TEXTURE_H_
