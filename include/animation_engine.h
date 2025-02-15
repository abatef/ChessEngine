#ifndef _ANIMATION_ENGINE_H_
#define _ANIMATION_ENGINE_H_

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

#include "piece.h"
#include "renderer.h"

class AnimationEngine {
   private:
    sf::RenderWindow& mWindow;
    sf::RenderTexture mAnimationSurface;
    Renderer& mRenderer;
    sf::Clock mClock;
    bool mIsMoving;
    const float kMovementDuration = 0.0001f;
    const float kStepFactor = 0.7f;
    std::vector<sf::Vector2f> plotLine(sf::Vector2f pStart, sf::Vector2f pEnd);

   public:
    AnimationEngine(Renderer& pRenderer);
    void animateMovement(Piece::PiecePtr pPiece, sf::Vector2f pSource, sf::Vector2f pTarget);
    bool isMoving() const;
};

#endif
