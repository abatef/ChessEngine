#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "board.h"
#include "square.h"
class Renderer {
   private:
    sf::RenderWindow m_Window;
    const sf::Color kHighlightColor = sf::Color(238, 238, 210, 250);

   public:
    Renderer();
    void drawSquare(Square::SquarePtr);
    void drawBoard(Board::BoardPtr);
    void update();
    void setDrawFlag();
    bool isRunning() const;
    sf::RenderWindow &getWindow();

    bool m_DrawFlag = true;
};

#endif