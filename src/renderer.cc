#include "renderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>

#include "board.h"
#include "piece.h"

Renderer::Renderer() : m_Window(sf::VideoMode(800, 800), "Chess") {}

bool Renderer::isRunning() const { return m_Window.isOpen(); }

sf::RenderWindow& Renderer::getWindow() { return m_Window; }

void Renderer::drawBoard(Board& board) {
    auto squares = board.getSquares();

    sf::RectangleShape rect;
    rect.setSize(sf::Vector2f(100.f, 100.f));
    for (auto& sl : squares) {
        for (auto& s : sl) {
            int x = s->getX() * 100.f;
            int y = s->getY() * 100.f;
            sf::Vector2f pos(x, y);
            rect.setPosition(pos);
            rect.setFillColor((s->getSquareColor() == EPieceColor::BLACK) ? sf::Color(118, 150, 86)
                                                                          : sf::Color::White);
            m_Window.draw(rect);
            if (s->isOccupied()) {
                s->getOccupier()->getSprite().setPosition(pos);
                m_Window.draw(s->getOccupier()->getSprite());
            }
        }
    }
}

void Renderer::update() { m_Window.display(); }