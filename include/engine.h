#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <set>

#include "animation_engine.h"
#include "board.h"
#include "input_handler.h"
#include "piece.h"
#include "renderer.h"
#include "square.h"

class Engine {
   private:
    Renderer m_Renderer;
    InputDispatcher m_Handler;
    Board::BoardPtr m_Board;
    sf::Clock m_Clock;
    AnimationEngine m_AnimationEngine;
    std::set<Square::SquarePtr> m_PossibleMoves;
    const float kMovementDuration = 3.f;
    bool m_IsMoving;

   private:
    void generatePawnMoves();
    void generateBishopMoves();
    void generateKnightMoves();
    void generateKingMoves();
    void generateQueenMoves();
    void generateRookMoves();
    void generateUsingCoords(std::vector<std::pair<int, int>> coords);

   public:
    Engine();
    Engine(InputDispatcher input);
    void handleInput();
    void loop();
    void selectSquare(Square::SquarePtr square);
    void proccessMove(Square::SquarePtr square);
    void movePiece(Piece::PiecePtr occupier, Square::SquarePtr targetSquare);
    void capturePiece(Piece::PiecePtr occupier, Square::SquarePtr targetSquare);
    void switchSelection(Square::SquarePtr currentSquare);
    void deselectSquare();
    void generatePossibleMoves();
    void clearHighlights();
    void highlightSquares();
    bool isLegalMove(Piece::PiecePtr, Square::SquarePtr);

    static Square::SquarePtr m_SelectedSquare;
};

#endif