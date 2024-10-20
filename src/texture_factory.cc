#include "texture_factory.h"

#include <SFML/Graphics/Texture.hpp>

const std::string path = "./assets/textures/";

sf::Texture &TextureFactory::getTexture(EPieceColor color, EPieceType type) {
    switch (color) {
        case EPieceColor::WHITE:
            switch (type) {
                case EPieceType::PAWN:
                    return TextureFactory::white_pawn_;
                case EPieceType::KNIGHT:
                    return TextureFactory::white_knight_;
                case EPieceType::BISHOP:
                    return TextureFactory::white_bishop_;
                case EPieceType::ROOK:
                    return TextureFactory::white_rook_;
                case EPieceType::QUEEN:
                    return TextureFactory::white_queen_;
                case EPieceType::KING:
                    return TextureFactory::white_king_;
            }
            break;
        case EPieceColor::BLACK:
            switch (type) {
                case EPieceType::PAWN:
                    return TextureFactory::black_pawn_;
                case EPieceType::KNIGHT:
                    return TextureFactory::black_knight_;
                case EPieceType::BISHOP:
                    return TextureFactory::black_bishop_;
                case EPieceType::ROOK:
                    return TextureFactory::black_rook_;
                case EPieceType::QUEEN:
                    return TextureFactory::black_queen_;
                case EPieceType::KING:
                    return TextureFactory::black_king_;
            }
            break;
        default:
            break;
    }
    static sf::Texture default_texture;
    return default_texture;
}

sf::Texture TextureFactory::white_pawn_ = TextureFactory::LoadTexture(path + "white-pawn.png");
sf::Texture TextureFactory::white_bishop_ = TextureFactory::LoadTexture(path + "white-bishop.png");
sf::Texture TextureFactory::white_king_ = TextureFactory::LoadTexture(path + "white-king.png");
sf::Texture TextureFactory::white_knight_ = TextureFactory::LoadTexture(path + "white-knight.png");
sf::Texture TextureFactory::white_rook_ = TextureFactory::LoadTexture(path + "white-rook.png");
sf::Texture TextureFactory::white_queen_ = TextureFactory::LoadTexture(path + "white-queen.png");

sf::Texture TextureFactory::black_pawn_ = TextureFactory::LoadTexture(path + "black-pawn.png");
sf::Texture TextureFactory::black_bishop_ = TextureFactory::LoadTexture(path + "black-bishop.png");
sf::Texture TextureFactory::black_king_ = TextureFactory::LoadTexture(path + "black-king.png");
sf::Texture TextureFactory::black_knight_ = TextureFactory::LoadTexture(path + "black-knight.png");
sf::Texture TextureFactory::black_rook_ = TextureFactory::LoadTexture(path + "black-rook.png");
sf::Texture TextureFactory::black_queen_ = TextureFactory::LoadTexture(path + "black-queen.png");
