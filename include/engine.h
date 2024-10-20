#ifndef _ENGINE_H_
#define _ENGINE_H_

#include <SFML/Graphics/RenderWindow.hpp>

#include "board.h"
#include "input_handler.h"
#include "renderer.h"
#include "square.h"

class Engine {
   private:
    Renderer m_Renderer;
    InputHandler m_Handler;
    Board m_Board;

   public:
    Engine();
    Engine(InputHandler input);
    void handleInput();
    void loop();
};

#endif