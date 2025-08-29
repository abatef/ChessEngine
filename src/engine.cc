#include "engine.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <iterator>
#include <memory>
#include <ostream>
#include <random>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "board.h"
#include "common.h"
#include "imgui.h"
#include "input_handler.h"
#include "piece.h"
#include "renderer.h"
#include "square.h"

static Piece::PiecePtr sSelectedPiece = nullptr;
static bool sRulesDisabled = true;
static bool sMoveHistoryShown = false;
static bool sPieceInfoShown = false;
static bool sMoveGeneration = false;
static bool sAiMoveGeneration = false;
static bool sAnimationEnabled = false;

bool isRulesDisabled() {
#ifdef IMGUI_MODE
    return sRulesDisabled;
#else
    return false;
#endif
}

bool isMoveHistoryShown() {
#ifdef IMGUI_MODE
    return sMoveHistoryShown;
#else
    return false;
#endif
}

bool isPieceInfoShown() {
#ifdef IMGUI_MODE
    return sPieceInfoShown;
#else
    return false;
#endif
}

bool isMoveGenerationEnabled() {
#ifdef IMGUI_MODE
    return sMoveGeneration;
#else
    return true;
#endif
}

bool isAiMoveGenerationEnabled() {
#ifdef IMGUI_MODE
    return sAiMoveGeneration;
#else
    return true;
#endif
}

bool isAnimationEnabled() {
#ifdef IMGUI_MODE
    return sAnimationEnabled;
#else
    return true;
#endif
}

Engine::Engine()
    : mInputDispatcher(mRenderer.getWindow())
    , mAnimationEngine(mRenderer)
    , mBoard(std::make_shared<Board>())
    , mGameMode(GameMode::SINGLE)
    , rng(rd()) {
    mBoard->init();
    Player *wPlayer = new Player(EPieceColor::WHITE);
    Player *bPlayer = new Player(EPieceColor::BLACK);
    wPlayer->mNext = bPlayer;
    bPlayer->mNext = wPlayer;
    mCurrentPlayer = wPlayer;
}

Engine::Engine(InputDispatcher pInputDispatcher)
    : Engine() {}

const sf::Color GREEN_SQUARE(118, 150, 86);

Square::SquarePtr Engine::mSelectedSquare = nullptr;

void Engine::resetEngine() {
    mBoard = std::make_shared<Board>();
    mBoard->init();
    if (mCurrentPlayer->mPlayerColor != EPieceColor::WHITE) {
        mCurrentPlayer = mCurrentPlayer->mNext;
    }
}

void Engine::switchPlayers() {
    auto king = findKing(mCurrentPlayer->mPlayerColor);
    king->mSquare->deSelect();
    mCurrentPlayer = mCurrentPlayer->mNext;
    mInputDispatcher.enableLocalInput();
    checkForCheckmate();
    if (mCurrentPlayer->mPlayerColor == EPieceColor::BLACK &&
        (mGameMode == GameMode::SINGLE || mGameMode == GameMode::ONLINE) &&
        isAiMoveGenerationEnabled()) {
        mInputDispatcher.disableLocalInput();
        makeBestMove();
        mInputDispatcher.enableLocalInput();
        mCurrentPlayer = mCurrentPlayer->mNext;
        return;
    }
}

void Engine::deselectSquare() {
    if (mSelectedSquare != nullptr) {
        mSelectedSquare->deSelect();
    }
    mSelectedSquare = nullptr;
    clearHighlights();
}

void Engine::copyMoves(std::vector<Square::SquarePtr> pMoves) {
    std::copy(pMoves.begin(), pMoves.end(), std::inserter(mLegalMoves, mLegalMoves.end()));
}

void Engine::selectSquare(Square::SquarePtr pCurrentSquare) {
    if (pCurrentSquare->isOccupied()) {
        if (pCurrentSquare->getOccupier()->getColor() == mCurrentPlayer->mPlayerColor ||
            isRulesDisabled()) {
            mSelectedSquare = pCurrentSquare;
            sSelectedPiece = mSelectedSquare->getOccupier();
            pCurrentSquare->select();
            if (isMoveGenerationEnabled()) {
                copyMoves(std::move(generatePossibleMoves(pCurrentSquare->getOccupier())));
                highlightSquares();
            }
            return;
        }
        return;
    }
}

void Engine::switchSelection(Square::SquarePtr pCurrentSquare) {
    clearHighlights();
    mSelectedSquare->deSelect();       // Deselect previous
    mSelectedSquare = pCurrentSquare;  // Select new
    sSelectedPiece = mSelectedSquare->getOccupier();
    pCurrentSquare->select();
    if (isMoveGenerationEnabled()) {
        copyMoves(std::move(generatePossibleMoves(mSelectedSquare->getOccupier())));
        highlightSquares();
    }
}

bool Engine::isLegalMove(Piece::PiecePtr pOccupier, Square::SquarePtr pTargetSquare) {
    if (mLegalMoves.empty()) {
        return false;
    }
    if (mLegalMoves.count(pTargetSquare)) {
        return true;
    }

    return false;
}

void Engine::movePiece(Piece::PiecePtr pOccupier, Square::SquarePtr pTargetSquare) {
    if (!isLegalMove(pOccupier, pTargetSquare) && !isRulesDisabled()) {
        deselectSquare();
        return;
    }
    auto startPos = mSelectedSquare->getPostion();
    auto targetPos = pTargetSquare->getPostion();
    bool isInCheck = isPlayerInCheck(mCurrentPlayer->mPlayerColor);
    Move move(pOccupier, nullptr, pOccupier->mSquare, pTargetSquare);
    if (mMoveHistory.empty()) {
        move = Move(pOccupier, nullptr, pOccupier->mSquare, pTargetSquare);
    } else if (mMoveHistory.top().mOpponent == nullptr) {
        move = Move(pOccupier, nullptr, pOccupier->mSquare, pTargetSquare);
    }
    if (pOccupier->mType != EPieceType::KING) {
        if (wouldExposeKing(move)) {
            deselectSquare();
            auto king = findKing(mCurrentPlayer->mPlayerColor);
            king->mSquare->select();
            return;
        }
    }
    mMoveHistory.push(move);
    if (pOccupier->mType == EPieceType::KING) {
        pOccupier->mSquare->deSelect();
    }
    mSelectedSquare->clear();
    pOccupier->setSquare(pTargetSquare);
    pOccupier->mMovedBefore = true;
    if (isAnimationEnabled()) {
        mAnimationEngine.animateMovement(pOccupier, startPos, targetPos);
    }
    pTargetSquare->setOccupier(pOccupier);
    deselectSquare();
    switchPlayers();
}

void Engine::capturePiece(Piece::PiecePtr pOccupier, Square::SquarePtr pTargetSquare) {
    if (!isLegalMove(pOccupier, pTargetSquare) && !isRulesDisabled()) {
        deselectSquare();
        return;
    }
    Square::SquarePtr tempTargetSquare = pTargetSquare;
    bool isEnPassantMove = false;
    if (pOccupier->mType == EPieceType::PAWN) {
        Square::SquarePtr enPassant = Engine::isEnPassant(pOccupier);
        if (pTargetSquare == enPassant) {
            isEnPassantMove = true;
            int x = pTargetSquare->getX();
            int y = pTargetSquare->getY();
            int v;
            if (pOccupier->mColor == EPieceColor::WHITE) {
                v = -1;
            } else {
                v = 1;
            }
            pTargetSquare = pTargetSquare->mBoard->squareAt({x, y + v});
            mLegalMoves.insert(pTargetSquare);
        }
    }
    auto opponent = tempTargetSquare->getOccupier();
    Move move(pOccupier, opponent, pOccupier->mSquare, pTargetSquare);
    if (isEnPassantMove) {
        move.mMoveType = MoveType::EN_PASSANT;
    }
    mMoveHistory.push(move);
    tempTargetSquare->clear();
    movePiece(pOccupier, pTargetSquare);
    opponent->deOccupy();
}

void Engine::proccessMove(Square::SquarePtr pCurrentSquare) {
    if (pCurrentSquare == mSelectedSquare) {
        deselectSquare();
        return;
    }

    if (mSelectedSquare->isOccupied()) {
        auto occupier = mSelectedSquare->getOccupier();
        if (!pCurrentSquare->isOccupied()) {
            movePiece(occupier, pCurrentSquare);
        } else {
            if (pCurrentSquare->getOccupier()->getColor() !=
                mSelectedSquare->getOccupier()->getColor()) {
                capturePiece(occupier, pCurrentSquare);
            } else {
                switchSelection(pCurrentSquare);
            }
        }
    }
}

void Engine::handleInput() {
    InputObject io = mInputDispatcher.captureInput();
    if (io.mType == ActionType::NONE) {
        return;
    }
    mRenderer.mDrawFlag = true;
    if (io.mType == ActionType::PRESS) {
        sf::Vector2i target = io.action.mTarget;
        auto currentSquare = mBoard->selectSquare(target);
        if (mSelectedSquare == nullptr) {
            selectSquare(currentSquare);
        } else {
            proccessMove(currentSquare);
        }
    }
}

std::vector<Square::SquarePtr> Engine::generatePossibleMoves(Piece::PiecePtr pPiece) {
    EPieceType selectedPieceType = pPiece->getType();
    switch (selectedPieceType) {
        case EPieceType::KING: return generateKingMoves(pPiece);
        case EPieceType::PAWN: return generatePawnMoves(pPiece);
        case EPieceType::ROOK: return generateRookMoves(pPiece);
        case EPieceType::BISHOP: return generateBishopMoves(pPiece);
        case EPieceType::QUEEN: return generateQueenMoves(pPiece);
        case EPieceType::KNIGHT: return generateKnightMoves(pPiece);
    }
}

std::vector<Square::SquarePtr> Engine::generatePawnMoves(Piece::PiecePtr pPiece) {
    std::vector<Square::SquarePtr> moves;
    sf::Vector2f position = pPiece->mSquare->getPostion();
    EPieceColor pieceColor = pPiece->getColor();

    int forward = (pieceColor == EPieceColor::WHITE) ? -1 : 1;
    int startRow = (pieceColor == EPieceColor::WHITE) ? 6 : 1;

    // Single forward move
    sf::Vector2i forwardMove(position.x, position.y + forward);
    if (forwardMove.y >= 0 && forwardMove.y < 8) {
        Square::SquarePtr forwardSquare = pPiece->mSquare->mBoard->squareAt(forwardMove);
        if (!forwardSquare->isOccupied()) {
            moves.push_back(forwardSquare);

            // Double forward move
            if (!pPiece->mMovedBefore) {
                sf::Vector2i doubleForwardMove(position.x, position.y + 2 * forward);
                Square::SquarePtr doubleForwardSquare =
                    pPiece->mSquare->mBoard->squareAt(doubleForwardMove);
                if (doubleForwardSquare && !doubleForwardSquare->isOccupied()) {
                    moves.push_back(doubleForwardSquare);
                }
            }
        }
    }

    // Diagonal captures
    for (int dx : {-1, 1}) {
        sf::Vector2i diagonalMove(position.x + dx, position.y + forward);
        if (diagonalMove.x >= 0 && diagonalMove.x < 8 && diagonalMove.y >= 0 &&
            diagonalMove.y < 8) {
            Square::SquarePtr diagonalSquare = pPiece->mSquare->mBoard->squareAt(diagonalMove);
            if (diagonalSquare->isOccupied() &&
                diagonalSquare->getOccupier()->getColor() != pieceColor) {
                moves.push_back(diagonalSquare);
            }
        }
    }

    Square::SquarePtr enPassant = Engine::isEnPassant(pPiece);
    if (enPassant) {
        moves.push_back(enPassant);
    }
    return moves;
}

Square::SquarePtr Engine::isEnPassant(Piece::PiecePtr pPawn) {
    sf::Vector2f pos = pPawn->mSquare->getPostion();
    EPieceColor pawnColor = pPawn->getColor();

    int enPassantRank = (pawnColor == EPieceColor::WHITE) ? 3 : 4;
    if (pPawn->mSquare->getY() != enPassantRank) {
        return nullptr;
    }

    Square::SquarePtr adjLeft = mBoard->squareAt({int(pos.x - 1), int(pos.y)});
    Square::SquarePtr adjRight = mBoard->squareAt({int(pos.x + 1), int(pos.y)});
    if ((!adjLeft || !adjLeft->isOccupied()) && (!adjRight || !adjRight->isOccupied())) {
        return nullptr;
    }
    if (mMoveHistory.empty()) {
        return nullptr;
    }

    Move lastMove = mMoveHistory.top();
    if (lastMove.mOccupier->mType != EPieceType::PAWN) {
        return nullptr;
    }

    int nSquares = std::abs(lastMove.mFrom->getY() - lastMove.mTo->getY());
    if (nSquares != 2) {
        return nullptr;
    }

    if (adjLeft && lastMove.mTo == adjLeft) {
        return adjLeft;
    }
    if (adjRight && lastMove.mTo == adjRight) {
        return adjRight;
    }

    return nullptr;
}

std::vector<Square::SquarePtr> Engine::generateUsingCoords(
    Piece::PiecePtr pPiece, const std::vector<std::pair<int, int>> &pCoords) {
    std::vector<Square::SquarePtr> moves;
    sf::Vector2f position = pPiece->mSquare->getPostion();
    EPieceColor color = pPiece->mSquare->getOccupier()->getColor();

    // Iterate over the direction pairs
    for (auto &[x, y] : pCoords) {
        sf::Vector2i newPosition(position.x, position.y);

        // Continue moving in the direction while within bounds
        while (true) {
            newPosition.x += x;
            newPosition.y += y;

            // Check for board boundaries
            if (newPosition.x < 0 || newPosition.x >= 8 || newPosition.y < 0 ||
                newPosition.y >= 8) {
                break;
            }

            Square::SquarePtr adjacentSquare = pPiece->mSquare->mBoard->squareAt(newPosition);

            // If square is occupied
            if (adjacentSquare->isOccupied()) {
                // If the occupier is the same color, stop
                if (adjacentSquare->getOccupier()->getColor() == color) {
                    break;
                }
                // If the occupier is of the opposite color, this is a valid move
                moves.push_back(adjacentSquare);
                break;  // No further moves in this direction
            }

            // If square is empty, it's a valid move
            moves.push_back(adjacentSquare);
        }
    }
    return moves;
}

std::vector<Square::SquarePtr> Engine::generateRookMoves(Piece::PiecePtr pPiece) {
    std::vector<std::pair<int, int>> coords{
        {0,  -1}, // Move up
        {0,  +1}, // Move down
        {+1, 0 }, // Move right
        {-1, 0 }  // Move left
    };

    return generateUsingCoords(pPiece, coords);
}

std::vector<Square::SquarePtr> Engine::generateBishopMoves(Piece::PiecePtr pPiece) {
    std::vector<std::pair<int, int>> coords{
        {+1, -1}, // Move diagonally up-left
        {-1, -1}, // Move diagonally up-right
        {+1, +1}, // Move diagonally down-left
        {-1, +1}  // Move diagonally down-right
    };

    return generateUsingCoords(pPiece, coords);
}

std::vector<Square::SquarePtr> Engine::generateQueenMoves(Piece::PiecePtr pPiece) {
    std::vector<Square::SquarePtr> moves;

    auto b = generateBishopMoves(pPiece);
    auto r = generateRookMoves(pPiece);
    moves.insert(moves.end(), b.begin(), b.end());
    moves.insert(moves.end(), r.begin(), r.end());

    return moves;
}

std::vector<Square::SquarePtr> Engine::generateMovesForPiece(
    Piece::PiecePtr pPiece, const std::vector<sf::Vector2i> &directions) {
    std::vector<Square::SquarePtr> moves;
    sf::Vector2f position = pPiece->mSquare->getPostion();
    EPieceColor color = pPiece->getColor();

    for (const auto &dir : directions) {
        sf::Vector2i newPosition(position.x + dir.x, position.y + dir.y);

        // Check if within bounds
        if (newPosition.x >= 0 && newPosition.x < 8 && newPosition.y >= 0 && newPosition.y < 8) {
            Square::SquarePtr adjacent = pPiece->mSquare->mBoard->squareAt(newPosition);

            // If square is unoccupied or occupied by the opponent's piece, add to legal moves
            if (!adjacent->isOccupied() || adjacent->getOccupier()->getColor() != color) {
                moves.push_back(adjacent);
            }
        }
    }
    return moves;
}

std::vector<Square::SquarePtr> Engine::generateKingMoves(Piece::PiecePtr pPiece) {
    std::vector<sf::Vector2i> coords{
        {0,  -1},
        {0,  +1},
        {-1, 0 },
        {+1, 0 },
        {-1, -1},
        {-1, +1},
        {+1, -1},
        {+1, +1}
    };

    return generateMovesForPiece(pPiece, coords);
}

// Generate Knight moves (L-shaped movement)
std::vector<Square::SquarePtr> Engine::generateKnightMoves(Piece::PiecePtr pPiece) {
    std::vector<sf::Vector2i> coords{
        {-2, -1},
        {-2, +1},
        {+2, -1},
        {+2, +1},
        {+1, +2},
        {-1, +2},
        {+1, -2},
        {-1, -2}
    };

    return generateMovesForPiece(pPiece, coords);
}

void Engine::clearHighlights() {
    for (auto &sq : mLegalMoves) {
        sq->clearHighlight();
    }
    mLegalMoves.clear();
}

void Engine::highlightSquares() {
    for (auto &sq : mLegalMoves) {
        sq->highlight();
    }
}

#ifdef IMGUI_MODE
void Engine::handleImGui() {
    // ImGui::SFML::Update(mRenderer.getWindow(), mRenderer.getClock().restart());
    int flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_BordersV |
                ImGuiTableFlags_BordersH;

    // ImGui::EndTable();

    ImGui::Begin("Engine Options");
    ImGui::Text("Local Input: %s", mInputDispatcher.isLocalInputEnabled() ? "Enabled" : "Disabled");
    ImGui::Text("Current Player %s",
                (mCurrentPlayer->mPlayerColor == EPieceColor::WHITE ? "White" : "Black"));
    ImGui::SameLine();
    if (ImGui::Button("Switch Player")) {
        switchPlayers();
    }

    ImGui::Checkbox("Disable Rules", &sRulesDisabled);
    ImGui::Checkbox("Moves History", &sMoveHistoryShown);
    ImGui::Checkbox("Show Piece Info", &sPieceInfoShown);
    ImGui::Checkbox("Enable Move Generation", &sMoveGeneration);
    ImGui::Checkbox("Enable AI", &sAiMoveGeneration);
    ImGui::Checkbox("Enable Animation", &sAnimationEnabled);
    if (ImGui::Button("Undo Last Move")) {
        undoMove();
        switchPlayers();
    }

    ImGui::End();

    if (sMoveHistoryShown) {
        if (ImGui::BeginTable("Move History", 5, flags)) {
            {
                ImGui::TableSetupColumn("Piece Name");
                ImGui::TableSetupColumn("at");
                ImGui::TableSetupColumn("to");
                ImGui::TableSetupColumn("captures");
                ImGui::TableSetupColumn("at");
                ImGui::TableHeadersRow();
            }

            std::stack<Move> st = mMoveHistory;

            while (!st.empty()) {
                auto move = st.top();
                st.pop();
                auto pieceName = move.mOccupier->getName();
                auto pieceFrom = move.mFrom->getPostion();
                auto pieceTo = move.mTo->getPostion();

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted(pieceName.c_str());
                ImGui::TableSetColumnIndex(1);
                std::string from = ("X: " + std::to_string(int(pieceFrom.x)) +
                                    " Y: " + std::to_string(int(pieceFrom.y)));
                ImGui::TextUnformatted(from.c_str());
                ImGui::TableSetColumnIndex(2);
                std::string to = ("X: " + std::to_string(int(pieceTo.x)) +
                                  " Y: " + std::to_string(int(pieceTo.y)));
                ImGui::TextUnformatted(to.c_str());
                ImGui::TableSetColumnIndex(3);
                if (move.mOpponent != nullptr) {
                    auto opponent = move.mOpponent->getName();
                    ImGui::TextUnformatted(opponent.c_str());
                    ImGui::TableSetColumnIndex(4);
                    ImGui::TextUnformatted(to.c_str());
                }
            }

            ImGui::EndTable();
        }
    }

    if (sPieceInfoShown) {
        ImGui::Begin("Piece Info");
        if (sSelectedPiece != nullptr) {
            ImGui::Text("Position: ");
            ImGui::SameLine();
            auto pos = sSelectedPiece->mSquare->getPostion();
            ImGui::Text("X: %0.f", pos.x);
            ImGui::SameLine();
            ImGui::Text("Y: %0.f", pos.y);
            ImGui::Separator();
            ImGui::Text("Piece Type: %s", sSelectedPiece->getName().c_str());
            ImGui::Separator();
            ImGui::Text("Piece Value %d", getPieceValue(sSelectedPiece->getType()));
            ImGui::Separator();
            ImGui::Checkbox("Moved Before", &sSelectedPiece->mMovedBefore);
            ImGui::Separator();
        }
        ImGui::End();
    }

    ImGui::SFML::Render(mRenderer.getWindow());
}
#endif

void Engine::loop() {
    while (mRenderer.isRunning()) {
        handleInput();
        mRenderer.drawBoard(mBoard, false);
#ifdef IMGUI_MODE
        handleImGui();
#endif
        mRenderer.update();
        sf::sleep(sf::milliseconds(10));
    }
}

Piece::PiecePtr Engine::findKing(EPieceColor pColor) const {
    if (pColor == EPieceColor::WHITE) {
        return mBoard->getPieces()[28];
    }
    return mBoard->getPieces()[29];
}

std::vector<Piece::PiecePtr> Engine::getOpponents(EPieceColor pColor) const {
    std::vector<Piece::PiecePtr> opponents;
    for (auto &p : mBoard->getPieces()) {
        if (p->mColor != pColor && p->mSquare) {
            opponents.push_back(p);
        }
    }
    return opponents;
}

bool Engine::isPlayerInCheck(EPieceColor pColor) {
    Piece::PiecePtr king = findKing(pColor);
    if (!king) return false;

    Square::SquarePtr kingSquare = king->mSquare;
    // kingSquare->deSelect();
    auto opponenets = getOpponents(pColor);

    for (auto &p : opponenets) {
        auto moves = generatePossibleMoves(p);
        if (std::find(moves.begin(), moves.end(), kingSquare) != moves.end()) {
            return true;
        }
    }
    return false;
}

bool Engine::wouldExposeKing(Move &m) {
    makeMove(m);
    bool exposed = isPlayerInCheck(mCurrentPlayer->mPlayerColor);
    undoMove();
    return exposed;
}

void Engine::checkForCheckmate() {
    if (!isPlayerInCheck(mCurrentPlayer->mPlayerColor)) {
        return;
    }
    auto king = findKing(mCurrentPlayer->mPlayerColor);
    auto moves = generateAllPossibleMoves(mCurrentPlayer->mPlayerColor);

    for (auto &m : moves) {
        if (!wouldExposeKing(m)) {
            return;
        }
    }

    declareCheckmate();
    endGame();
}

void Engine::declareCheckmate() { std::cout << "Checkmate" << std::endl; }

void Engine::endGame() { resetEngine(); }

std::vector<Move> Engine::generateAllPossibleMoves(EPieceColor pPieceColor) {
    mLegalMoves.clear();
    std::vector<Move> moves;
    for (auto &p : mBoard->getPieces()) {
        if (p->getColor() == pPieceColor && p->mSquare) {
            // std::cout << "Generating Moves for: " << p->getName() << std::endl;
            auto legalMoves = generatePossibleMoves(p);
            for (auto &s : legalMoves) {
                Piece::PiecePtr opponent = nullptr;
                Move move(p, opponent, p->mSquare, s);
                if (s->isOccupied() && s->getOccupier()->getType() == EPieceType::KING) {
                    continue;
                }
                if (s->isOccupied()) {
                    opponent = s->getOccupier();
                    move.mMoveType = MoveType::CAPTURE;
                }
                move.mOpponent = opponent;
                moves.push_back(move);
            }
        }
    }
    return moves;
}

int Engine::getPieceValue(EPieceType pType) const {
    switch (pType) {
        case EPieceType::PAWN: return 100;
        case EPieceType::KNIGHT: return 320;
        case EPieceType::BISHOP: return 330;
        case EPieceType::ROOK: return 500;
        case EPieceType::QUEEN: return 900;
        case EPieceType::KING: return 20000;
        default: return 0;
    }
}

int Engine::evaluateBoard() const {
    int score = 0;
    // Material evaluation
    for (auto &p : mBoard->getPieces()) {
        if (p->mSquare) {
            int pieceValue = getPieceValue(p->getType());
            score += (p->getColor() == EPieceColor::WHITE) ? pieceValue : -pieceValue;

            // Pawn advancement bonus
            if (p->getType() == EPieceType::PAWN) {
                int rank = p->mSquare->getY();
                score += (p->getColor() == EPieceColor::WHITE ? rank : 7 - rank) * 10;
            }

            // Position-based evaluation
            if (p->getType() == EPieceType::KNIGHT || p->getType() == EPieceType::BISHOP) {
                // Bonus for developed pieces
                int rank = p->mSquare->getY();
                int file = p->mSquare->getX();
                int centerDistance = std::abs(3.5 - file) + std::abs(3.5 - rank);
                int developmentBonus = (8 - centerDistance) * 5;
                score +=
                    (p->getColor() == EPieceColor::WHITE) ? developmentBonus : -developmentBonus;
            }
        }
    }

    return score;
}

void printMove(Move pMove, bool undo) {
    if (undo) {
        std::cout << "Undoing Move..." << "\n";
    } else {
        std::cout << "Making Move..." << "\n";
    }
    std::cout << pMove.mOccupier->getName() << " From: " << pMove.mFrom->getX() << " "
              << pMove.mFrom->getY();
    std::cout << " To: " << pMove.mTo->getX() << " " << pMove.mTo->getY() << " ";
    if (pMove.mOpponent) {
        std::cout << pMove.mOpponent->getName() << std::endl;
    } else {
        std::cout << std::endl;
    }
}

void Engine::makeMove(Move pMove) {
    // printMove(pMove, false);
    pMove.mFirstMove = false;
    if (!pMove.mOccupier->mMovedBefore) {
        pMove.mFirstMove = true;
        pMove.mOccupier->mMovedBefore = true;
    }
    mMoveHistory.push(pMove);
    auto occupier = pMove.mOccupier;
    auto opponent = pMove.mOpponent;
    auto from = pMove.mFrom;
    auto to = pMove.mTo;

    if (to->isOccupied()) {
        to->clear();
    }
    from->clear();
    occupier->setSquare(to);
    // occupier->mMovedBefore = true;
    to->setOccupier(occupier);
}

void Engine::undoMove() {
    auto pMove = mMoveHistory.top();
    // printMove(pMove, true);
    mMoveHistory.pop();
    if (pMove.mFirstMove) {
        pMove.mOccupier->mMovedBefore = false;
    }
    auto occupier = pMove.mOccupier;
    auto opponent = pMove.mOpponent;
    auto from = pMove.mFrom;
    auto to = pMove.mTo;
    to->clear();
    if (opponent) {
        to->setOccupier(opponent);
        opponent->setSquare(to);
    }
    from->clear();
    from->setOccupier(occupier);
    occupier->setSquare(from);
}

int Engine::minimax(int pDepth, int pAlpha, int pBeta, bool pIsMaximizing) {
    if (pDepth == 0) {
        return evaluateBoard();
    }

    std::vector<Move> moves =
        generateAllPossibleMoves(pIsMaximizing ? EPieceColor::WHITE : EPieceColor::BLACK);

    // Sort moves to improve alpha-beta pruning
    std::sort(moves.begin(), moves.end(), [&](const Move &m1, const Move &m2) {
        if (m1.mMoveType == MoveType::CAPTURE && m2.mMoveType != MoveType::CAPTURE) return true;
        if (m1.mMoveType == MoveType::CAPTURE && m2.mMoveType == MoveType::CAPTURE) {
            return getPieceValue(m1.mOpponent->getType()) > getPieceValue(m2.mOpponent->getType());
        }
        return false;
    });

    if (pIsMaximizing) {
        int maxEval = -100000;
        for (auto &move : moves) {
            if (wouldExposeKing(move)) {
                continue;
            }
            makeMove(move);
            int eval = minimax(pDepth - 1, pAlpha, pBeta, false);
            undoMove();

            maxEval = std::max(maxEval, eval);
            pAlpha = std::max(pAlpha, eval);
            if (pBeta <= pAlpha) break;
        }
        return maxEval;
    } else {
        int minEval = 100000;
        for (auto &move : moves) {
            if (wouldExposeKing(move)) {
                continue;
            }
            makeMove(move);
            int eval = minimax(pDepth - 1, pAlpha, pBeta, true);
            undoMove();

            minEval = std::min(minEval, eval);
            pBeta = std::min(pBeta, eval);
            if (pBeta <= pAlpha) break;
        }
        return minEval;
    }
}

void Engine::makeBestMove() {
    int bestMoveValue = -100000;
    Move bestMove;
    auto possibleMoves = generateAllPossibleMoves(EPieceColor::BLACK);
    std::vector<Move> moves;

    for (auto &move : possibleMoves) {
        if (wouldExposeKing(move)) {
            continue;
        }
        makeMove(move);
        int moveValue = minimax(3, -100000, 100000, false);

        // Add capture bonus BEFORE comparison
        if (move.mMoveType == MoveType::CAPTURE) {
            moveValue += getPieceValue(move.mOpponent->getType());  // Increase capture incentive
        }

        undoMove();

        if (moveValue > bestMoveValue) {
            moves.clear();
            moves.push_back(move);
            bestMoveValue = moveValue;
            bestMove = move;
        } else if (moveValue == bestMoveValue) {
            moves.push_back(move);
        }
    }

    if (!moves.empty()) {
        // Prioritize captures and higher-value captures
        std::sort(moves.begin(), moves.end(), [&](const Move &m1, const Move &m2) {
            // First priority: captures vs non-captures
            if (m1.mMoveType == MoveType::CAPTURE && m2.mMoveType != MoveType::CAPTURE) return true;
            if (m1.mMoveType != MoveType::CAPTURE && m2.mMoveType == MoveType::CAPTURE)
                return false;

            // Second priority: value of captured piece
            if (m1.mMoveType == MoveType::CAPTURE && m2.mMoveType == MoveType::CAPTURE) {
                return getPieceValue(m1.mOpponent->getType()) >
                       getPieceValue(m2.mOpponent->getType());
            }

            // For non-captures, maintain some randomness
            return false;
        });

        // Select best capture if available, otherwise random move
        if (moves[0].mMoveType == MoveType::CAPTURE) {
            bestMove = moves[0];  // Take the highest-value capture
        } else {
            std::uniform_int_distribution<> dist(0, moves.size() - 1);
            bestMove = moves[dist(rng)];
        }

        if (wouldExposeKing(bestMove)) {
            for (auto &m : moves) {
                if (!wouldExposeKing(m)) {
                    bestMove = m;
                    break;
                }
            }
        }
    }
    bestMove.mFrom->clear();
    bestMove.mOccupier->setSquare(bestMove.mFrom);
    if (isAnimationEnabled()) {
        mAnimationEngine.animateMovement(bestMove.mOccupier, bestMove.mFrom->getPostion(),
                                         bestMove.mTo->getPostion());
    }
    bestMove.mFrom->setOccupier(bestMove.mOccupier);
    makeMove(bestMove);
}

Move::Move(Piece::PiecePtr pPiece, Piece::PiecePtr pOpponent, Square::SquarePtr pFrom,
           Square::SquarePtr pTo)
    : mOccupier(pPiece)
    , mFrom(pFrom)
    , mTo(pTo)
    , mOpponent(pOpponent) {}
