#ifndef _PIECE_H_
#define _PIECE_H_
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <memory>

enum class EPieceColor { BLACK, WHITE };
enum class EPieceType { PAWN, ROOK, BISHOP, QUEEN, KING, KNIGHT };

class Square;

class Piece : public std::enable_shared_from_this<Piece> {
   public:
    using PiecePtr = std::shared_ptr<Piece>;
    std::shared_ptr<Square> mSquare;

    EPieceColor mColor;
    EPieceType mType;
    sf::Sprite mSprite;
    bool mMovedBefore;

   public:
    Piece(EPieceType pType, EPieceColor pColor);
    virtual ~Piece() = default;
    bool movedBefore() const;
    void setFirstMove();
    void setSpritePosition(sf::Vector2f pPosition);
    EPieceColor getColor() const;
    EPieceType getType() const;
    sf::Vector2f getSpritePosition();
    sf::Sprite &getSprite();
    void deOccupy();
    void setSquare(std::shared_ptr<Square> pSquare);
    std::string getName() const;
};

#endif