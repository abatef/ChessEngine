#include "renderer.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <SFML/Window/WindowStyle.hpp>

#include "SFML/System/Clock.hpp"
#include "board.h"
#include "common.h"
#include "piece.h"

Renderer::Renderer()
    : mWindow(sf::VideoMode(800, 800), "Chess", sf::Style::Titlebar | sf::Style::Close) {
    // mWindow.setFramerateLimit(60);
#ifdef IMGUI_MODE
    ImGui::SFML::Init(mWindow);
#endif
}

Renderer::~Renderer() {
    mWindow.close();
#ifdef IMGUI_MODE
    ImGui::SFML::Shutdown();
#endif
}

bool Renderer::isRunning() const { return mWindow.isOpen(); }

sf::RenderWindow& Renderer::getWindow() { return mWindow; }
sf::Clock& Renderer::getClock() { return mDeltaClock; }

void Renderer::drawBoard(Board::BoardPtr pBoard, bool pAnimating) {
#ifndef IMGUI_MODE
    if (!mDrawFlag) {
        return;
    }
#endif

    mWindow.clear(sf::Color::Black);

#ifdef IMGUI_MODE
    if (!pAnimating) {
        ImGui::SFML::Update(mWindow, mDeltaClock.restart());
    }

#endif
    auto squares = pBoard->getSquares();
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
            if (s->isSelected() || s->isHighlighted()) {
                float outlineThickness = 2.f;
                rect.setSize(
                    sf::Vector2f(100.f - 2 * outlineThickness, 100.f - 2 * outlineThickness));
                rect.setPosition(pos.x + outlineThickness, pos.y + outlineThickness);
                if (s->isSelected()) {
                    rect.setOutlineColor(sf::Color::Red);
                } else if (s->isHighlighted()) {
                    rect.setOutlineColor(sf::Color::Blue);
                }
                rect.setOutlineThickness(outlineThickness);
            } else {
                rect.setSize(sf::Vector2f(100.f, 100.f));
                rect.setOutlineThickness(0.f);
            }
            mWindow.draw(rect);
            if (s->isOccupied()) {
                s->getOccupier()->getSprite().setPosition(pos);
                mWindow.draw(s->getOccupier()->getSprite());
            }
        }
    }
}

void Renderer::update() {
#ifndef IMGUI_MODE
    if (!mDrawFlag) {
        return;
    }
#endif
#ifdef IMGUI_MODE
    ImGui::SFML::Render(mWindow);
#endif
    mWindow.display();
    mDrawFlag = false;
}