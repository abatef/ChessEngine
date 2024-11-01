#include "engine.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <cstdio>
#include <iostream>
#include <memory>
#include <ostream>
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
    , mGameMode(GameMode::SINGLE) {
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
    auto opponent = pTargetSquare->getOccupier();
    Move move(pOccupier, opponent, pOccupier->mSquare, pTargetSquare);
    mMoveHistory.push(move);
    pTargetSquare->clear();
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
                if (s->isOccupied()) {
                    opponent = s->getOccupier();
                }
                moves.emplace_back(p, opponent, p->mSquare, s);
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
    for (auto &p : mBoard->getPieces()) {
        if (p->mSquare) {
            int pieceValue = getPieceValue(p->getType());
            score += (p->getColor() == EPieceColor::WHITE) ? pieceValue : -pieceValue;
        }
    }
    // if (score != 0) {
    //     std::cout << "Total Score: " << score << std::endl;
    // }
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
    if (pIsMaximizing) {
        int maxEval = -100000;
        auto possibleMoves = generateAllPossibleMoves(EPieceColor::WHITE);
        for (auto &move : possibleMoves) {
            makeMove(move);
            int eval = minimax(pDepth - 1, pAlpha, pBeta, false);
            undoMove();
            maxEval = std::max(maxEval, eval);
            // std::cout << "MAX: " << maxEval << std::endl;
            pAlpha = std::max(pAlpha, eval);
            if (pBeta <= pAlpha) {
                break;
            }
        }
        return maxEval;
    } else {
        int minEval = 100000;
        auto possibleMoves = generateAllPossibleMoves(EPieceColor::BLACK);
        for (auto &move : possibleMoves) {
            makeMove(move);
            int eval = minimax(pDepth - 1, pAlpha, pBeta, true);
            undoMove();

            minEval = std::min(minEval, eval);
            // std::cout << "MIN: " << minEval << std::endl;

            pBeta = std::min(pBeta, eval);
            if (pBeta <= pAlpha) {
                break;
            }
        }
        return minEval;
    }
}

void Engine::makeBestMove() {
    int bestMoveValue = -100000;
    Move bestMove;
    auto possibleMoves = generateAllPossibleMoves(EPieceColor::BLACK);
    for (auto &move : possibleMoves) {
        makeMove(move);
        int moveValue = minimax(3, -100000, 100000, false);
        undoMove();
        if (moveValue > bestMoveValue) {
            bestMoveValue = moveValue;
            bestMove = move;
        }
    }
    bestMove.mFrom->clear();
    bestMove.mOccupier->setSquare(bestMove.mFrom);
    mAnimationEngine.animateMovement(bestMove.mOccupier, bestMove.mFrom->getPostion(),
                                     bestMove.mTo->getPostion());
    bestMove.mFrom->setOccupier(bestMove.mOccupier);
    return makeMove(bestMove);
}

Move::Move(Piece::PiecePtr pPiece, Piece::PiecePtr pOpponent, Square::SquarePtr pFrom,
           Square::SquarePtr pTo)
    : mOccupier(pPiece)
    , mFrom(pFrom)
    , mTo(pTo)
    , mOpponent(pOpponent) {}
