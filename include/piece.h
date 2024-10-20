#ifndef _PIECE_H_
#define _PIECE_H_
#include <SFML/Graphics.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <memory>
#include <utility>
#include <vector>

enum class EPieceColor { BLACK, WHITE };
enum class EPieceType { PAWN, ROOK, BISHOP, QUEEN, KING, KNIGHT };

class Square;

class Piece : public std::enable_shared_from_this<Piece> {
   public:
    using PiecePtr = std::shared_ptr<Piece>;

   protected:
    EPieceColor _color;
    EPieceType _type;
    sf::Sprite _sprite;
    std::vector<std::pair<int, int>> _moves;
    std::shared_ptr<Square> _square;

   public:
    Piece(EPieceType, EPieceColor);
    virtual ~Piece() = default;
    virtual void generateMoves() = 0;
    bool canMoveTo(std::shared_ptr<Square>) const;
    bool moveTo(std::shared_ptr<Square>);
    EPieceColor getColor() const;
    EPieceType getType() const;
    sf::Vector2f getSpritePosition();
    void setSpritePosition(sf::Vector2f pos);
    sf::Sprite &getSprite();
    void deOccupy();
    void setSquare(std::shared_ptr<Square>);
};

#endif