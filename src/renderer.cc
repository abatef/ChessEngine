#include "renderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>

#include "board.h"
#include "piece.h"

Renderer::Renderer() : m_Window(sf::VideoMode(800, 800), "Chess") {
    //   << std::endl;
}

bool Renderer::isRunning() const { return m_Window.isOpen(); }

sf::RenderWindow& Renderer::getWindow() { return m_Window; }

void Renderer::drawBoard(Board& board) {
    if (!m_DrawFlag) {
        return;
    }

    m_Window.clear(sf::Color::Black);
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
            if (s->isSelected()) {
                float outlineThickness = 2.f;
                rect.setSize(
                    sf::Vector2f(100.f - 2 * outlineThickness, 100.f - 2 * outlineThickness));
                rect.setPosition(pos.x + outlineThickness, pos.y + outlineThickness);
                rect.setOutlineColor(sf::Color::Red);
                rect.setOutlineThickness(outlineThickness);
            } else {
                rect.setSize(sf::Vector2f(100.f, 100.f));
                rect.setOutlineThickness(0.f);
            }
            m_Window.draw(rect);
            if (s->isOccupied()) {
                s->getOccupier()->getSprite().setPosition(pos);
                m_Window.draw(s->getOccupier()->getSprite());
            }
        }
    }
}

void Renderer::update() {
    if (!m_DrawFlag) {
        return;
    }
    m_Window.display();
    m_DrawFlag = false;
}