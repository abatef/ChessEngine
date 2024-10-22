#include "board.h"

#include <SFML/System/Vector2.hpp>
#include <iostream>
#include <memory>
#include <vector>

#include "piece.h"
#include "square.h"

void Board::placePiece(int x, int y, EPieceType type, EPieceColor color) {
    Piece::PiecePtr piece = std::make_shared<Piece>(type, color);
    m_Squares[x][y]->setOccupier(piece);
}

bool Board::init() {
    std::cout << "Board Init" << std::endl;
    // Initialize Empty Squares
    for (int i = 0; i < 8; i++) {
        std::vector<Square::SquarePtr> row;
        EPieceColor color = (i % 2 == 0) ? EPieceColor::WHITE : EPieceColor::BLACK;

        for (int j = 0; j < 8; j++) {
            Square::SquarePtr sqr = std::make_shared<Square>(i, j, color);
            sqr->setBoard(shared_from_this());
            row.push_back(sqr);
            color = (color == EPieceColor::WHITE) ? EPieceColor::BLACK : EPieceColor::WHITE;
        }

        m_Squares.push_back(row);
    }
    // Initial Pieces Positions
    // PAWNS
    for (int i = 0; i < 8; i++) {
        placePiece(i, 1, EPieceType::PAWN, EPieceColor::BLACK);
        placePiece(i, 6, EPieceType::PAWN, EPieceColor::WHITE);
    }
    // White Bishops
    placePiece(2, 0, EPieceType::BISHOP, EPieceColor::BLACK);
    placePiece(5, 0, EPieceType::BISHOP, EPieceColor::BLACK);
    // Black Bishops
    placePiece(2, 7, EPieceType::BISHOP, EPieceColor::WHITE);
    placePiece(5, 7, EPieceType::BISHOP, EPieceColor::WHITE);
    // White Rooks
    placePiece(0, 7, EPieceType::ROOK, EPieceColor::WHITE);
    placePiece(7, 7, EPieceType::ROOK, EPieceColor::WHITE);
    // Black Rooks
    placePiece(0, 0, EPieceType::ROOK, EPieceColor::BLACK);
    placePiece(7, 0, EPieceType::ROOK, EPieceColor::BLACK);
    // White Knights
    placePiece(1, 7, EPieceType::KNIGHT, EPieceColor::WHITE);
    placePiece(6, 7, EPieceType::KNIGHT, EPieceColor::WHITE);
    // Black Knights
    placePiece(1, 0, EPieceType::KNIGHT, EPieceColor::BLACK);
    placePiece(6, 0, EPieceType::KNIGHT, EPieceColor::BLACK);
    // White King
    placePiece(4, 7, EPieceType::KING, EPieceColor::WHITE);
    // Black King
    placePiece(4, 0, EPieceType::KING, EPieceColor::BLACK);
    // White Queen
    placePiece(3, 7, EPieceType::QUEEN, EPieceColor::WHITE);
    // Black Queen
    placePiece(3, 0, EPieceType::QUEEN, EPieceColor::BLACK);
    return true;
}

Board::BoardT &Board::getSquares() { return m_Squares; }

Square::SquarePtr Board::selectSquare(sf::Vector2i squarePosition) {
    int x = squarePosition.x / 100;
    int y = squarePosition.y / 100;
    return m_Squares[x][y];
}

Square::SquarePtr Board::squareAt(sf::Vector2i squareIndex) {
    return m_Squares[squareIndex.x][squareIndex.y];
}