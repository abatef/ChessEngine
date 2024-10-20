#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include <SFML/Graphics/RenderWindow.hpp>
class InputHandler {
   private:
    sf::RenderWindow &m_Window;

   public:
    InputHandler(sf::RenderWindow &window);
    void handleInput();
};

#endif