#include "piece_texture.h"

#include <SFML/Graphics/Texture.hpp>

const std::string path = "./assets/textures/";

sf::Texture &PieceTexture::getTexture(EPieceColor color, EPieceType type) {
    switch (color) {
        case EPieceColor::WHITE:
            switch (type) {
                case EPieceType::PAWN:
                    return PieceTexture::white_pawn_;
                case EPieceType::KNIGHT:
                    return PieceTexture::white_knight_;
                case EPieceType::BISHOP:
                    return PieceTexture::white_bishop_;
                case EPieceType::ROOK:
                    return PieceTexture::white_rook_;
                case EPieceType::QUEEN:
                    return PieceTexture::white_queen_;
                case EPieceType::KING:
                    return PieceTexture::white_king_;
            }
            break;
        case EPieceColor::BLACK:
            switch (type) {
                case EPieceType::PAWN:
                    return PieceTexture::black_pawn_;
                case EPieceType::KNIGHT:
                    return PieceTexture::black_knight_;
                case EPieceType::BISHOP:
                    return PieceTexture::black_bishop_;
                case EPieceType::ROOK:
                    return PieceTexture::black_rook_;
                case EPieceType::QUEEN:
                    return PieceTexture::black_queen_;
                case EPieceType::KING:
                    return PieceTexture::black_king_;
            }
            break;
        default:
            break;
    }
    static sf::Texture default_texture;
    return default_texture;
}

sf::Texture PieceTexture::white_pawn_ = PieceTexture::LoadTexture(path + "white-pawn.png");
sf::Texture PieceTexture::white_bishop_ = PieceTexture::LoadTexture(path + "white-bishop.png");
sf::Texture PieceTexture::white_king_ = PieceTexture::LoadTexture(path + "white-king.png");
sf::Texture PieceTexture::white_knight_ = PieceTexture::LoadTexture(path + "white-knight.png");
sf::Texture PieceTexture::white_rook_ = PieceTexture::LoadTexture(path + "white-rook.png");
sf::Texture PieceTexture::white_queen_ = PieceTexture::LoadTexture(path + "white-queen.png");

sf::Texture PieceTexture::black_pawn_ = PieceTexture::LoadTexture(path + "black-pawn.png");
sf::Texture PieceTexture::black_bishop_ = PieceTexture::LoadTexture(path + "black-bishop.png");
sf::Texture PieceTexture::black_king_ = PieceTexture::LoadTexture(path + "black-king.png");
sf::Texture PieceTexture::black_knight_ = PieceTexture::LoadTexture(path + "black-knight.png");
sf::Texture PieceTexture::black_rook_ = PieceTexture::LoadTexture(path + "black-rook.png");
sf::Texture PieceTexture::black_queen_ = PieceTexture::LoadTexture(path + "black-queen.png");
