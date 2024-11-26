#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <random>
#include <set>
#include <stack>
#include <vector>

#include "animation_engine.h"
#include "board.h"
#include "common.h"
#include "input_handler.h"
#include "piece.h"
#include "renderer.h"
#include "square.h"

enum class GameMode { SINGLE, ONLINE, LOCAL };

enum class MoveType { NORMAL, CAPTURE, EN_PASSANT };

struct Move {
    Piece::PiecePtr mOccupier = nullptr;
    Piece::PiecePtr mOpponent = nullptr;
    Square::SquarePtr mFrom = nullptr;
    Square::SquarePtr mTo = nullptr;
    MoveType mMoveType = MoveType::NORMAL;
    bool mFirstMove;
    Move() = default;
    Move(Piece::PiecePtr pPiece, Piece::PiecePtr pOpponent, Square::SquarePtr pFrom,
         Square::SquarePtr pTo);
};

struct Player {
    Player() = default;
    Player(EPieceColor color)
        : mPlayerColor(color) {}
    EPieceColor mPlayerColor;
    Player* mNext;
};

class Engine {
   private:
    Renderer mRenderer;
    InputDispatcher mInputDispatcher;
    Board::BoardPtr mBoard;
    sf::Clock mClock;
    AnimationEngine mAnimationEngine;
    std::set<Square::SquarePtr> mLegalMoves;
    std::set<Square::SquarePtr> mCachedMoves;
    GameMode mGameMode;
    std::stack<Move> mMoveHistory;
    Player* mCurrentPlayer;
    std::random_device rd;
    std::mt19937 rng;
    const float kMovementDuration = 3.f;

   private:
    std::vector<Square::SquarePtr> generatePawnMoves(Piece::PiecePtr pPiece);
    Square::SquarePtr isEnPassant(Piece::PiecePtr pPawn);
    std::vector<Square::SquarePtr> generateBishopMoves(Piece::PiecePtr pPiece);
    std::vector<Square::SquarePtr> generateKnightMoves(Piece::PiecePtr pPiece);
    std::vector<Square::SquarePtr> generateKingMoves(Piece::PiecePtr pPiece);
    std::vector<Square::SquarePtr> generateQueenMoves(Piece::PiecePtr pPiece);
    std::vector<Square::SquarePtr> generateRookMoves(Piece::PiecePtr pPiece);
    std::vector<Square::SquarePtr> generateUsingCoords(
        Piece::PiecePtr pPiece, const std::vector<std::pair<int, int>>& pCoords);
    std::vector<Square::SquarePtr> generateMovesForPiece(
        Piece::PiecePtr pPiece, const std::vector<sf::Vector2i>& directions);
    void copyMoves(std::vector<Square::SquarePtr> pMoves);
    void switchPlayers();
    void makeMove(Move pMove);
    void undoMove();
    std::vector<Move> generateAllPossibleMoves(EPieceColor pPieceColor);
    int getPieceValue(EPieceType pType) const;
    int evaluateBoard() const;
    int minimax(int pDepth, int pAlpha, int pBeta, bool pIsMaximizing);
    void makeBestMove();
#ifdef IMGUI_MODE
    void handleImGui();
#endif
    Piece::PiecePtr findKing(EPieceColor pColor) const;
    std::vector<Piece::PiecePtr> getOpponents(EPieceColor pColor) const;
    bool isPlayerInCheck(EPieceColor pColor);
    bool wouldExposeKing(Move& m);
    void checkForCheckmate();
    void declareCheckmate();
    void endGame();

   public:
    Engine();
    Engine(InputDispatcher pInput);
    void handleInput();
    void loop();
    void selectSquare(Square::SquarePtr pSquare);
    void proccessMove(Square::SquarePtr pSquare);
    void movePiece(Piece::PiecePtr pOccupier, Square::SquarePtr pTargetSquare);
    void capturePiece(Piece::PiecePtr pOccupier, Square::SquarePtr pTargetSquare);
    void switchSelection(Square::SquarePtr pCurrentSquare);
    void deselectSquare();
    std::vector<Square::SquarePtr> generatePossibleMoves(Piece::PiecePtr pPiece);
    void clearHighlights();
    void highlightSquares();
    bool isLegalMove(Piece::PiecePtr pOccupier, Square::SquarePtr pTargetSquare);

    static Square::SquarePtr mSelectedSquare;
};

#endif