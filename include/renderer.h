#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <SFML/Graphics/RenderWindow.hpp>

#include "board.h"
#include "square.h"
class Renderer {
   private:
    sf::RenderWindow m_Window;

   public:
    Renderer();
    void drawSquare(Square::SquarePtr);
    void drawBoard(Board &);
    void update();
    bool isRunning() const;
    sf::RenderWindow &getWindow();
};

#endif