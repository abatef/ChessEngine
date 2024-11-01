#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "SFML/System/Clock.hpp"
#include "board.h"
#include "square.h"
class Renderer {
   private:
    sf::RenderWindow mWindow;
    const sf::Color kHighlightColor = sf::Color(238, 238, 210, 250);
    sf::Clock mDeltaClock;

   public:
    Renderer();
    ~Renderer();
    void drawSquare(Square::SquarePtr pSquare);
    void drawBoard(Board::BoardPtr pBoard, bool pAnimating);
    void update();
    void setDrawFlag();
    bool isRunning() const;
    sf::RenderWindow &getWindow();
    sf::Clock &getClock();

    bool mDrawFlag = true;
};

#endif