#include "animation_engine.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/System/Sleep.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <iostream>

#include "common.h"
#include "imgui-SFML.h"
#include "imgui.h"
#include "piece.h"
#include "renderer.h"

AnimationEngine::AnimationEngine(Renderer &pRenderer)
    : mWindow(pRenderer.getWindow())
    , mRenderer(pRenderer)
    , mAnimationSurface()
    , mIsMoving(true) {
    mAnimationSurface.create(800, 800);
}

std::vector<sf::Vector2f> AnimationEngine::plotLine(sf::Vector2f pStart, sf::Vector2f pEnd) {
    std::vector<sf::Vector2f> points{};

    float dx = pEnd.x - pStart.x;
    float dy = pEnd.y - pStart.y;

    // If the line is vertical
    if (dx == 0) {
        float step = (pEnd.y > pStart.y) ? kStepFactor : -kStepFactor;
        for (float y = pStart.y; (step > 0 ? y <= pEnd.y : y >= pEnd.y); y += step) {
            points.push_back(sf::Vector2f(pStart.x, y));
        }
    }
    // If the line is horizontal
    else if (dy == 0) {
        float step = (pEnd.x > pStart.x) ? kStepFactor : -kStepFactor;
        for (float x = pStart.x; (step > 0 ? x <= pEnd.x : x >= pEnd.x); x += step) {
            points.push_back(sf::Vector2f(x, pStart.y));
        }
    }
    // General case (diagonal)
    else {
        float slope = dy / dx;
        float step = (pEnd.x > pStart.x) ? kStepFactor : -kStepFactor;

        for (float x = pStart.x; (step > 0 ? x <= pEnd.x : x >= pEnd.x); x += step) {
            float y = pStart.y + slope * (x - pStart.x);
            points.push_back(sf::Vector2f(x, y));
        }
    }

    return std::move(points);
}

void AnimationEngine::animateMovement(Piece::PiecePtr pPiece, sf::Vector2f pSource,
                                      sf::Vector2f pTarget) {
    // Generate the points along the line from start to target
    auto points = plotLine(pSource * 100.f, pTarget * 100.f);

    // Retrieve the sprite to animate
    auto sprite = pPiece->getSprite();

    // Setup clock for timing the animation
    mClock.restart();
    float animationTime = 0.f;
    size_t currentPointIndex = 0;

    // Start the animation loop
    while (currentPointIndex < points.size()) {
        float deltaTime = mClock.restart().asSeconds();
        animationTime += deltaTime;  // the time passed since last move
        // ImGui::SFML::Update(mRenderer.getWindow(), mRenderer.getClock().restart());
        // Check if enough time has passed to move to the next point
        // ImGui::Text("Piece: %s", pPiece->getName().c_str());
        if (animationTime >= kMovementDuration) {
            // Get the current point to move to
            auto &[x, y] = points[currentPointIndex];
            sprite.setPosition(x, y);
            currentPointIndex++;
            animationTime = 0.f;  // Reset animation timer
        }
        // Clear the off-screen surface and window
        mAnimationSurface.clear(sf::Color(255, 255, 255, 0));
        mRenderer.drawBoard(pPiece->mSquare->mBoard, true);
        // ImGui::SFML::Update(mRenderer.getWindow(), mRenderer.getClock().restart());
        // Draw the sprite at the new position
        mAnimationSurface.draw(sprite);
        mAnimationSurface.display();

        // Draw the animation surface onto the window
        mWindow.draw(sf::Sprite(mAnimationSurface.getTexture()));
        mWindow.display();
    }
}

bool AnimationEngine::isMoving() const { return mIsMoving; }