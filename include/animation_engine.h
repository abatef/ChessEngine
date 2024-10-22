#ifndef _ANIMATION_ENGINE_H_
#define _ANIMATION_ENGINE_H_

#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Vector2.hpp>
#include <vector>

// TODO https://www.sfml-dev.org/documentation/2.1/classsf_1_1RenderTexture.php

#include "piece.h"
#include "renderer.h"
class AnimationEngine {
   private:
    sf::RenderWindow& m_Window;
    sf::RenderTexture m_AnimationSurface;
    Renderer& m_Renderer;
    sf::Clock m_Clock;
    bool m_IsMoving;
    const float kMovementDuration = 3.f;
    float m_ElapsedTime;
    std::vector<sf::Vector2f> plotLine(sf::Vector2f start, sf::Vector2f end);

   public:
    AnimationEngine(Renderer&);
    void move(Piece::PiecePtr, sf::Vector2f, sf::Vector2f);
    bool isMoving() const;
};

#endif