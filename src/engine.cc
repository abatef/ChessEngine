#include "engine.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>
#include <memory>
#include <ostream>
#include <utility>
#include <vector>

#include "board.h"
#include "input_handler.h"
#include "piece.h"
#include "player.h"
#include "renderer.h"
#include "square.h"

Engine::Engine()
    : m_Handler(m_Renderer.getWindow()),
      m_AnimationEngine(m_Renderer),
      m_Board(std::make_shared<Board>()) {
    m_Board->init();
    Player *w_Player = new Player(EPieceColor::WHITE);
    Player *b_Player = new Player(EPieceColor::BLACK);
    w_Player->next = b_Player;
    b_Player->next = w_Player;
    m_CurrentPlayer = w_Player;
}

Engine::Engine(InputDispatcher handler)
    : m_Handler(m_Renderer.getWindow()), m_AnimationEngine((m_Renderer)) {}

const sf::Color GREEN_SQUARE(118, 150, 86);

Square::SquarePtr Engine::m_SelectedSquare = nullptr;

void Engine::deselectSquare() {
    if (m_SelectedSquare != nullptr) {
        m_SelectedSquare->deSelect();
    }
    m_SelectedSquare = nullptr;
    clearHighlights();
}

void Engine::selectSquare(Square::SquarePtr currentSquare) {
    if (currentSquare->isOccupied()) {
        if (currentSquare->getOccupier()->getColor() == m_CurrentPlayer->m_PlayerColor) {
            m_SelectedSquare = currentSquare;
            currentSquare->select();
            generatePossibleMoves();
            highlightSquares();
            return;
        }
        return;
    }
}

void Engine::switchSelection(Square::SquarePtr currentSquare) {
    clearHighlights();
    m_SelectedSquare->deSelect();      // Deselect previous
    m_SelectedSquare = currentSquare;  // Select new
    currentSquare->select();
    generatePossibleMoves();
    highlightSquares();
}

bool Engine::isLegalMove(Piece::PiecePtr occupier, Square::SquarePtr targetSquare) {
    if (m_PossibleMoves.empty()) {
        return false;
    }

    if (m_PossibleMoves.count(targetSquare)) {
        return true;
    }

    return false;
}

void Engine::movePiece(Piece::PiecePtr occupier, Square::SquarePtr targetSquare) {
    if (!isLegalMove(occupier, targetSquare)) {
        deselectSquare();
        return;
    }
    auto startPos = m_SelectedSquare->getPostion();
    auto targetPos = targetSquare->getPostion();
    m_SelectedSquare->clear();
    occupier->setSquare(targetSquare);
    occupier->setFirstMove();
    m_AnimationEngine.move(occupier, startPos, targetPos);
    targetSquare->setOccupier(occupier);
    deselectSquare();
    m_CurrentPlayer = m_CurrentPlayer->next;
}

void Engine::capturePiece(Piece::PiecePtr occupier, Square::SquarePtr targetSquare) {
    if (!isLegalMove(occupier, targetSquare)) {
        deselectSquare();
        return;
    }
    auto opponent = targetSquare->getOccupier();
    targetSquare->clear();
    movePiece(occupier, targetSquare);
    opponent->deOccupy();
}

void Engine::proccessMove(Square::SquarePtr currentSquare) {
    if (currentSquare == m_SelectedSquare) {
        deselectSquare();
        return;
    }

    if (m_SelectedSquare->isOccupied()) {
        auto occupier = m_SelectedSquare->getOccupier();
        if (!currentSquare->isOccupied()) {
            movePiece(occupier, currentSquare);
        } else {
            if (currentSquare->getOccupier()->getColor() !=
                m_SelectedSquare->getOccupier()->getColor()) {
                capturePiece(occupier, currentSquare);
            } else {
                switchSelection(currentSquare);
            }
        }
    }
}

void Engine::handleInput() {
    InputObject io = m_Handler.captureInput();
    if (io.type == ActionType::NONE) {
        return;
    }
    m_Renderer.m_DrawFlag = true;
    if (io.type == ActionType::PRESS) {
        sf::Vector2i target = io.action.target;
        std::cout << "SELECT ON X: " << io.action.target.x << " Y: " << io.action.target.y
                  << std::endl;
        auto currentSquare = m_Board->selectSquare(target);
        if (m_SelectedSquare == nullptr) {
            selectSquare(currentSquare);
        } else {
            proccessMove(currentSquare);
        }
    }
}

void Engine::generatePossibleMoves() {
    EPieceType selectedPieceType = m_SelectedSquare->getOccupier()->getType();
    switch (selectedPieceType) {
        case EPieceType::PAWN:
            generatePawnMoves();
            break;
        case EPieceType::ROOK:
            generateRookMoves();
            break;
        case EPieceType::BISHOP:
            generateBishopMoves();
            break;
        case EPieceType::QUEEN:
            generateQueenMoves();
            break;
        case EPieceType::KING:
            generateKingMoves();
            break;
        case EPieceType::KNIGHT:
            generateKnightMoves();
            break;
    }
}

// TODO: implement en passant and promotion
void Engine::generatePawnMoves() {
    sf::Vector2f position = m_SelectedSquare->getPostion();
    bool firstMove = m_SelectedSquare->getOccupier()->isFirstMove();
    EPieceColor pieceColor = m_SelectedSquare->getOccupier()->getColor();
    if (!firstMove) {
        if (pieceColor == EPieceColor::WHITE) {
            m_PossibleMoves.insert(
                m_SelectedSquare->m_Board->squareAt(sf::Vector2i(position.x, position.y - 1)));
            m_PossibleMoves.insert(
                m_SelectedSquare->m_Board->squareAt(sf::Vector2i(position.x, position.y - 2)));
            return;
        }
        m_PossibleMoves.insert(
            m_SelectedSquare->m_Board->squareAt(sf::Vector2i(position.x, position.y + 1)));
        m_PossibleMoves.insert(
            m_SelectedSquare->m_Board->squareAt(sf::Vector2i(position.x, position.y + 2)));
        return;
    }
    sf::Vector2i forwardMove;
    if (pieceColor == EPieceColor::WHITE) {
        forwardMove = sf::Vector2i(position.x, position.y - 1);
    } else {
        forwardMove = sf::Vector2i(position.x, position.y + 1);
    }
    Square::SquarePtr forwardSquare = m_SelectedSquare->m_Board->squareAt(forwardMove);
    if (!forwardSquare->isOccupied()) {
        m_PossibleMoves.insert(forwardSquare);
    }
    Square::SquarePtr adjacentLeft =
        m_SelectedSquare->m_Board->squareAt(sf::Vector2i(forwardMove.x - 1, forwardMove.y));
    Square::SquarePtr adjacentRight =
        m_SelectedSquare->m_Board->squareAt(sf::Vector2i(forwardMove.x + 1, forwardMove.y));

    if (adjacentLeft->isOccupied()) {
        m_PossibleMoves.insert(adjacentLeft);
    }
    if (adjacentRight->isOccupied()) {
        m_PossibleMoves.insert(adjacentRight);
    }
}

void Engine::generateUsingCoords(std::vector<std::pair<int, int>> coords) {
    sf::Vector2f position = m_SelectedSquare->getPostion();
    EPieceColor color = m_SelectedSquare->getOccupier()->getColor();

    for (auto &[x, y] : coords) {
        sf::Vector2i newPosition(position.x, position.y);
        while (newPosition.x + x >= 0 && newPosition.x + x < 8 && newPosition.y + y >= 0 &&
               newPosition.y + y < 8) {
            newPosition.x += x;
            newPosition.y += y;
            Square::SquarePtr adjacentSquare = m_SelectedSquare->m_Board->squareAt(newPosition);
            if (adjacentSquare->isOccupied()) {
                if (adjacentSquare->getOccupier()->getColor() == color) {
                    break;
                }
                m_PossibleMoves.insert(adjacentSquare);
                break;
            }
            m_PossibleMoves.insert(adjacentSquare);
        }
    }
}

void Engine::generateRookMoves() {
    sf::Vector2f position = m_SelectedSquare->getPostion();
    std::vector<std::pair<int, int>> coords{
        {0,  -1},
        {0,  +1},
        {+1, 0 },
        {-1, 0 }
    };

    generateUsingCoords(std::move(coords));
}

void Engine::generateBishopMoves() {
    sf::Vector2f position = m_SelectedSquare->getPostion();
    EPieceColor pieceColor = m_SelectedSquare->getOccupier()->getColor();

    std::vector<std::pair<int, int>> coords{
        {+1, -1},
        {-1, -1},
        {+1, +1},
        {-1, +1}
    };

    generateUsingCoords(std::move(coords));
}

void Engine::generateQueenMoves() {
    generateBishopMoves();
    generateRookMoves();
}

void Engine::generateKingMoves() {
    sf::Vector2f position = m_SelectedSquare->getPostion();
    EPieceColor color = m_SelectedSquare->getOccupier()->getColor();

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
            Square::SquarePtr adjacent = m_SelectedSquare->m_Board->squareAt(newPosition);
            if (!adjacent->isOccupied()) {
                m_PossibleMoves.insert(adjacent);
                continue;
            }
            if (adjacent->getOccupier()->getColor() != color) {
                if (adjacent->getOccupier()->getType() == EPieceType::KING) {
                    continue;
                }
                m_PossibleMoves.insert(adjacent);
            }
        }
    }
}

void Engine::generateKnightMoves() {
    sf::Vector2f position = m_SelectedSquare->getPostion();
    EPieceColor color = m_SelectedSquare->getOccupier()->getColor();

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
            Square::SquarePtr adjacent = m_SelectedSquare->m_Board->squareAt(newPosition);
            if (adjacent->isOccupied() && adjacent->getOccupier()->getColor() == color) {
                continue;
            }
            m_PossibleMoves.insert(adjacent);
        }
    }
}

void Engine::clearHighlights() {
    for (auto &sq : m_PossibleMoves) {
        sq->clearHighlight();
    }
    m_PossibleMoves.clear();
}

void Engine::highlightSquares() {
    for (auto &sq : m_PossibleMoves) {
        sq->highlight();
    }
}

void Engine::loop() {
    while (m_Renderer.isRunning()) {
        handleInput();
        m_Renderer.drawBoard(m_Board);
        m_Renderer.update();
        sf::sleep(sf::milliseconds(10));
    }
}