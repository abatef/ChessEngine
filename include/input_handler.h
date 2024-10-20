#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

enum class ActionType { NONE, PRESS, ENGINE };

typedef struct InputObject {
    ActionType type = ActionType::NONE;
    union action {
        sf::Vector2i target;
    } action;
} InputObject;

class InputDispatcher {
   private:
    sf::RenderWindow &m_Window;

   public:
    InputDispatcher(sf::RenderWindow &window);
    InputObject captureInput();
};

#endif