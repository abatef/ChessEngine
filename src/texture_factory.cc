#include "texture_factory.h"

#include <SFML/Graphics/Texture.hpp>

const std::string path = "/home/abatef/workspace/ChessEngine/assets/textures/";

sf::Texture &TextureFactory::getTexture(EPieceColor pColor, EPieceType pType) {
    switch (pColor) {
        case EPieceColor::WHITE:
            switch (pType) {
                case EPieceType::PAWN: return TextureFactory::mWhitePawn;
                case EPieceType::KNIGHT: return TextureFactory::mWhiteKnight;
                case EPieceType::BISHOP: return TextureFactory::mWhiteBishop;
                case EPieceType::ROOK: return TextureFactory::mWhiteRook;
                case EPieceType::QUEEN: return TextureFactory::mWhiteQueen;
                case EPieceType::KING: return TextureFactory::mWhiteKing;
            }
            break;
        case EPieceColor::BLACK:
            switch (pType) {
                case EPieceType::PAWN: return TextureFactory::mBlackPawn;
                case EPieceType::KNIGHT: return TextureFactory::mBlackKnight;
                case EPieceType::BISHOP: return TextureFactory::mBlackBishop;
                case EPieceType::ROOK: return TextureFactory::mBlackRook;
                case EPieceType::QUEEN: return TextureFactory::mBlackQueen;
                case EPieceType::KING: return TextureFactory::mBlackKing;
            }
            break;
        default: break;
    }
    static sf::Texture default_texture;
    return default_texture;
}

sf::Texture TextureFactory::mWhitePawn = TextureFactory::LoadTexture(path + "white-pawn.png");
sf::Texture TextureFactory::mWhiteBishop = TextureFactory::LoadTexture(path + "white-bishop.png");
sf::Texture TextureFactory::mWhiteKing = TextureFactory::LoadTexture(path + "white-king.png");
sf::Texture TextureFactory::mWhiteKnight = TextureFactory::LoadTexture(path + "white-knight.png");
sf::Texture TextureFactory::mWhiteRook = TextureFactory::LoadTexture(path + "white-rook.png");
sf::Texture TextureFactory::mWhiteQueen = TextureFactory::LoadTexture(path + "white-queen.png");

sf::Texture TextureFactory::mBlackPawn = TextureFactory::LoadTexture(path + "black-pawn.png");
sf::Texture TextureFactory::mBlackBishop = TextureFactory::LoadTexture(path + "black-bishop.png");
sf::Texture TextureFactory::mBlackKing = TextureFactory::LoadTexture(path + "black-king.png");
sf::Texture TextureFactory::mBlackKnight = TextureFactory::LoadTexture(path + "black-knight.png");
sf::Texture TextureFactory::mBlackRook = TextureFactory::LoadTexture(path + "black-rook.png");
sf::Texture TextureFactory::mBlackQueen = TextureFactory::LoadTexture(path + "black-queen.png");
