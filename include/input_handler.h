#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>

enum class ActionType { NONE, PRESS, ENGINE };

typedef struct InputObject {
    ActionType mType = ActionType::NONE;
    union action {
        sf::Vector2i mTarget;
    } action;
} InputObject;

class InputDispatcher {
   private:
    sf::RenderWindow &mWindow;
    bool mLocalInputEnabled;

   public:
    InputDispatcher(sf::RenderWindow &pWindow);
    InputObject captureInput();
    void enableLocalInput();
    void disableLocalInput();
    bool isLocalInputEnabled() const;
};

#endif