#include "animation_engine.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>

#include "piece.h"
#include "renderer.h"

AnimationEngine::AnimationEngine(Renderer& renderer)
    : m_Window(renderer.getWindow()),
      m_Renderer(renderer),
      m_AnimationSurface(),
      m_ElapsedTime(0),
      m_IsMoving(true) {
    m_AnimationSurface.create(800, 800);
}

std::vector<sf::Vector2f> AnimationEngine::plotLine(sf::Vector2f start, sf::Vector2f end) {
    std::vector<sf::Vector2f> points{};

    float dx = end.x - start.x;
    float dy = end.y - start.y;

    // If the line is vertical
    if (dx == 0) {
        float step = (end.y > start.y) ? 0.7f : -0.7f;
        for (float y = start.y; (step > 0 ? y <= end.y : y >= end.y); y += step) {
            points.push_back(sf::Vector2f(start.x, y));
        }
    }
    // If the line is horizontal
    else if (dy == 0) {
        float step = (end.x > start.x) ? 0.7f : -0.7f;
        for (float x = start.x; (step > 0 ? x <= end.x : x >= end.x); x += step) {
            points.push_back(sf::Vector2f(x, start.y));
        }
    }
    // General case (diagonal)
    else {
        float slope = dy / dx;
        float step = (end.x > start.x) ? 0.7f : -0.7f;

        for (float x = start.x; (step > 0 ? x <= end.x : x >= end.x); x += step) {
            float y = start.y + slope * (x - start.x);
            points.push_back(sf::Vector2f(x, y));
        }
    }

    std::cout << "Points: " << points.size() << std::endl;

    return points;
}

void AnimationEngine::move(Piece::PiecePtr piece, sf::Vector2f startPos, sf::Vector2f targetPos) {
    std::cout << "Animating" << std::endl;

    // Generate the points along the line from start to target
    auto points = plotLine(startPos * 100.f, targetPos * 100.f);

    // Retrieve the sprite to animate
    auto sprite = piece->getSprite();

    // Setup clock for timing the animation
    sf::Clock clock;
    float animationTime = 0.f;
    size_t currentPointIndex = 0;

    // Start the animation loop
    while (currentPointIndex < points.size()) {
        float deltaTime = clock.restart().asSeconds();
        animationTime += deltaTime;

        // Check if enough time has passed to move to the next point
        if (animationTime >= 0.0001f) {  // Adjust 0.01f for speed
            // Get the current point to move to
            auto [x, y] = points[currentPointIndex];
            sprite.setPosition(x, y);
            currentPointIndex++;
            animationTime = 0.f;  // Reset animation timer
        }

        // Clear the off-screen surface and window
        m_AnimationSurface.clear(sf::Color(255, 255, 255, 0));
        m_Renderer.drawBoard(piece->m_Square->m_Board);

        // Draw the sprite at the new position
        m_AnimationSurface.draw(sprite);
        m_AnimationSurface.display();

        // Draw the animation surface onto the window
        m_Window.draw(sf::Sprite(m_AnimationSurface.getTexture()));
        m_Window.display();

        // Sleep for a short time to control the frame rate
        // sf::sleep(sf::milliseconds(10));
    }
}

bool AnimationEngine::isMoving() const { return m_IsMoving; }