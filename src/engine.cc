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
static bool sRulesDisabled = false;
static bool sMoveHistoryShown = false;
static bool sPieceInfoShown = false;
static bool sMoveGenerationEnabled = false;

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

void Engine::switchPlayers() {
    mCurrentPlayer = mCurrentPlayer->mNext;
    mInputDispatcher.enableLocalInput();
    if (mCurrentPlayer->mPlayerColor == EPieceColor::BLACK &&
        (mGameMode == GameMode::SINGLE || mGameMode == GameMode::ONLINE) &&
        sMoveGenerationEnabled) {
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

void Engine::selectSquare(Square::SquarePtr pCurrentSquare) {
    if (pCurrentSquare->isOccupied()) {
        if (pCurrentSquare->getOccupier()->getColor() == mCurrentPlayer->mPlayerColor ||
            sRulesDisabled) {
            mSelectedSquare = pCurrentSquare;
            sSelectedPiece = mSelectedSquare->getOccupier();
            pCurrentSquare->select();
            generatePossibleMoves(pCurrentSquare->getOccupier());
            highlightSquares();
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
    generatePossibleMoves(mSelectedSquare->getOccupier());
    highlightSquares();
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
    if (!isLegalMove(pOccupier, pTargetSquare)) {
        deselectSquare();
        return;
    }
    auto startPos = mSelectedSquare->getPostion();
    auto targetPos = pTargetSquare->getPostion();
    if (mMoveHistory.empty()) {
        Move move(pOccupier, nullptr, pOccupier->mSquare, pTargetSquare);
        mMoveHistory.push(move);
    } else if (mMoveHistory.top().mOpponent == nullptr) {
        Move move(pOccupier, nullptr, pOccupier->mSquare, pTargetSquare);
        mMoveHistory.push(move);
    }
    mSelectedSquare->clear();
    pOccupier->setSquare(pTargetSquare);
    pOccupier->mMovedBefore = true;
    mAnimationEngine.animateMovement(pOccupier, startPos, targetPos);
    pTargetSquare->setOccupier(pOccupier);
    deselectSquare();
    switchPlayers();
}

void Engine::capturePiece(Piece::PiecePtr pOccupier, Square::SquarePtr pTargetSquare) {
    if (!isLegalMove(pOccupier, pTargetSquare)) {
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

void Engine::generatePossibleMoves(Piece::PiecePtr pPiece) {
    EPieceType selectedPieceType = pPiece->getType();
    switch (selectedPieceType) {
        case EPieceType::KING: generateKingMoves(pPiece); break;
        case EPieceType::PAWN: generatePawnMoves(pPiece); break;
        case EPieceType::ROOK: generateRookMoves(pPiece); break;
        case EPieceType::BISHOP: generateBishopMoves(pPiece); break;
        case EPieceType::QUEEN: generateQueenMoves(pPiece); break;
        case EPieceType::KNIGHT: generateKnightMoves(pPiece); break;
    }
}

// TODO: implement en passant
void Engine::generatePawnMoves(Piece::PiecePtr pPiece) {
    sf::Vector2f position = pPiece->mSquare->getPostion();
    EPieceColor pieceColor = pPiece->getColor();
    if (!pPiece->mMovedBefore) {
        if (pieceColor == EPieceColor::WHITE) {
            if (position.y - 1 >= 0) {
                auto s =
                    pPiece->mSquare->mBoard->squareAt(sf::Vector2i(position.x, position.y - 1));
                if (s->isOccupied()) {
                    if (s->getOccupier()->getColor() != pieceColor) {
                        mLegalMoves.insert(s);
                        return;
                    }
                    return;
                }
                mLegalMoves.insert(s);
                if (position.y - 2 >= 0) {
                    mLegalMoves.insert(pPiece->mSquare->mBoard->squareAt(
                        sf::Vector2i(position.x, position.y - 2)));
                }
            }
            return;
        }
        if (position.y + 1 < 8) {
            auto s = pPiece->mSquare->mBoard->squareAt(sf::Vector2i(position.x, position.y + 1));
            if (s->isOccupied()) {
                if (s->getOccupier()->getColor() != pieceColor) {
                    mLegalMoves.insert(s);
                    return;
                }
                return;
            }
            mLegalMoves.insert(s);
            if (position.y + 2 < 8) {
                mLegalMoves.insert(
                    pPiece->mSquare->mBoard->squareAt(sf::Vector2i(position.x, position.y + 2)));
            }
        }
        return;
    }
    sf::Vector2i forwardMove;
    if (pieceColor == EPieceColor::WHITE) {
        forwardMove = sf::Vector2i(position.x, position.y - 1);
    } else {
        forwardMove = sf::Vector2i(position.x, position.y + 1);
    }
    if ((forwardMove.y >= 0 && forwardMove.y < 8)) {
        Square::SquarePtr forwardSquare = pPiece->mSquare->mBoard->squareAt(forwardMove);
        if (!forwardSquare->isOccupied()) {
            mLegalMoves.insert(forwardSquare);
        }
        if (forwardMove.x - 1 >= 0) {
            Square::SquarePtr adjacentLeft =
                pPiece->mSquare->mBoard->squareAt(sf::Vector2i(forwardMove.x - 1, forwardMove.y));
            if (adjacentLeft->isOccupied() &&
                adjacentLeft->getOccupier()->getColor() != mCurrentPlayer->mPlayerColor) {
                mLegalMoves.insert(adjacentLeft);
            }
        }

        if (forwardMove.x + 1 < 8) {
            Square::SquarePtr adjacentRight =
                pPiece->mSquare->mBoard->squareAt(sf::Vector2i(forwardMove.x + 1, forwardMove.y));
            if (adjacentRight->isOccupied() &&
                adjacentRight->getOccupier()->getColor() != mCurrentPlayer->mPlayerColor) {
                mLegalMoves.insert(adjacentRight);
            }
        }
    }
    Square::SquarePtr enPassant = Engine::isEnPassant(pPiece);
    if (enPassant) {
        mLegalMoves.insert(enPassant);
    }
}

Square::SquarePtr Engine::isEnPassant(Piece::PiecePtr pPawn) {
    sf::Vector2f pos = pPawn->mSquare->getPostion();
    Square::SquarePtr adjLeft = mBoard->squareAt({int(pos.x - 1), int(pos.y)});
    Square::SquarePtr adjRight = mBoard->squareAt({int(pos.x + 1), int(pos.y)});
    if (pPawn->mSquare->getY() == 3 || pPawn->mSquare->getY() == 4) {
        if ((adjRight && adjRight->isOccupied()) || (adjLeft && adjLeft->isOccupied())) {
            Move m = mMoveHistory.top();
            if (m.mOccupier->mType != EPieceType::PAWN) {
                return nullptr;
            }
            int nSquares = std::abs(m.mFrom->getY() - m.mTo->getY());
            if ((m.mTo == adjLeft || m.mTo == adjRight) && nSquares == 2) {
                if (m.mTo == adjLeft) {
                    return adjLeft;
                } else if (m.mTo == adjRight) {
                    return adjRight;
                }
            }
        }
    }
    return nullptr;
}

void Engine::generateUsingCoords(Piece::PiecePtr pPiece, std::vector<std::pair<int, int>> pCoords) {
    sf::Vector2f position = pPiece->mSquare->getPostion();
    EPieceColor color = pPiece->mSquare->getOccupier()->getColor();

    for (auto &[x, y] : pCoords) {
        sf::Vector2i newPosition(position.x, position.y);
        while (newPosition.x + x >= 0 && newPosition.x + x < 8 && newPosition.y + y >= 0 &&
               newPosition.y + y < 8) {
            newPosition.x += x;
            newPosition.y += y;
            Square::SquarePtr adjacentSquare = pPiece->mSquare->mBoard->squareAt(newPosition);
            if (adjacentSquare->isOccupied()) {
                if (adjacentSquare->getOccupier()->getColor() == color) {
                    break;
                }
                mLegalMoves.insert(adjacentSquare);
                break;
            }
            mLegalMoves.insert(adjacentSquare);
        }
    }
}

void Engine::generateRookMoves(Piece::PiecePtr pPiece) {
    sf::Vector2f position = pPiece->mSquare->getPostion();
    std::vector<std::pair<int, int>> coords{
        {0,  -1},
        {0,  +1},
        {+1, 0 },
        {-1, 0 }
    };

    generateUsingCoords(pPiece, std::move(coords));
}

void Engine::generateBishopMoves(Piece::PiecePtr pPiece) {
    sf::Vector2f position = pPiece->mSquare->getPostion();

    std::vector<std::pair<int, int>> coords{
        {+1, -1},
        {-1, -1},
        {+1, +1},
        {-1, +1}
    };

    generateUsingCoords(pPiece, std::move(coords));
}

void Engine::generateQueenMoves(Piece::PiecePtr pPiece) {
    generateBishopMoves(pPiece);
    generateRookMoves(pPiece);
}

void Engine::generateKingMoves(Piece::PiecePtr pPiece) {
    sf::Vector2f position = pPiece->mSquare->getPostion();
    EPieceColor color = pPiece->getColor();

    std::vector<std::pair<int, int>> coords{
        {0,  -1},
        {0,  +1},
        {-1, 0 },
        {+1, 0 },
        {-1, -1},
        {-1, +1},
        {+1, -1},
        {+1, +1}
    };

    for (auto &[x, y] : coords) {
        if (position.x + x >= 0 && position.x + x < 8 && position.y + y >= 0 &&
            position.y + y < 8) {
            sf::Vector2i newPosition(position.x + x, position.y + y);
            Square::SquarePtr adjacent = pPiece->mSquare->mBoard->squareAt(newPosition);
            if (!adjacent->isOccupied()) {
                mLegalMoves.insert(adjacent);
                continue;
            }
            if (adjacent->getOccupier()->getColor() != color) {
                if (adjacent->getOccupier()->getType() == EPieceType::KING) {
                    continue;
                }
                mLegalMoves.insert(adjacent);
            }
        }
    }
}

void Engine::generateKnightMoves(Piece::PiecePtr pPiece) {
    sf::Vector2f position = pPiece->mSquare->getPostion();
    EPieceColor color = pPiece->getColor();

    std::vector<std::pair<int, int>> coords{
        {-2, -1},
        {-2, +1},
        {+2, -1},
        {+2, +1},
        {+1, +2},
        {-1, +2},
        {+1, -2},
        {-1, -2},
    };

    for (auto &[x, y] : coords) {
        if (position.x + x >= 0 && position.x + x < 8 && position.y + y >= 0 &&
            position.y + y < 8) {
            sf::Vector2i newPosition(position.x + x, position.y + y);
            Square::SquarePtr adjacent = pPiece->mSquare->mBoard->squareAt(newPosition);
            if (adjacent->isOccupied() && adjacent->getOccupier()->getColor() == color) {
                continue;
            }
            mLegalMoves.insert(adjacent);
        }
    }
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
    ImGui::Checkbox("Enable Move Generation", &sMoveGenerationEnabled);
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

std::vector<Move> Engine::generateAllPossibleMoves(EPieceColor pPieceColor) {
    mLegalMoves.clear();
    std::vector<Move> moves;
    for (auto &p : mBoard->getPieces()) {
        if (p->getColor() == pPieceColor && p->mSquare) {
            // std::cout << "Generating Moves for: " << p->getName() << std::endl;
            generatePossibleMoves(p);
            for (auto &s : mLegalMoves) {
                Piece::PiecePtr opponent = nullptr;
                Move move(p, opponent, p->mSquare, s);
                if (s->isOccupied()) {
                    opponent = s->getOccupier();
                    move.mMoveType = MoveType::CAPTURE;
                }
                move.mOpponent = opponent;
                moves.push_back(move);
            }
            mLegalMoves.clear();
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
        makeMove(move);
        int moveValue = minimax(3, -100000, 100000, false);

        // Add capture bonus BEFORE comparison
        if (move.mMoveType == MoveType::CAPTURE) {
            moveValue +=
                getPieceValue(move.mOpponent->getType()) * 2;  // Increase capture incentive
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
    }
    bestMove.mFrom->clear();
    bestMove.mOccupier->setSquare(bestMove.mFrom);
    mAnimationEngine.animateMovement(bestMove.mOccupier, bestMove.mFrom->getPostion(),
                                     bestMove.mTo->getPostion());
    bestMove.mFrom->setOccupier(bestMove.mOccupier);
    makeMove(bestMove);
}

Move::Move(Piece::PiecePtr pPiece, Piece::PiecePtr pOpponent, Square::SquarePtr pFrom,
           Square::SquarePtr pTo)
    : mOccupier(pPiece)
    , mFrom(pFrom)
    , mTo(pTo)
    , mOpponent(pOpponent) {}
