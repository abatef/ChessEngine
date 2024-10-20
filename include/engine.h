#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <SFML/Graphics/RenderWindow.hpp>

#include "board.h"
#include "input_handler.h"
#include "piece.h"
#include "renderer.h"
#include "square.h"

class Engine {
   private:
    Renderer m_Renderer;
    InputDispatcher m_Handler;
    Board m_Board;

   public:
    Engine();
    Engine(InputDispatcher input);
    void handleInput();
    void loop();
    static Square::SquarePtr selectedSquare;
};

#endif