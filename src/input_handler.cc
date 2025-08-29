#include "input_handler.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>

#include "common.h"

InputDispatcher::InputDispatcher(sf::RenderWindow& pWindow)
    : mWindow(pWindow)
    , mLocalInputEnabled(true) {}

InputObject InputDispatcher::captureInput() {
    sf::Event e;
    while (mWindow.pollEvent(e)) {
#ifdef IMGUI_MODE
        ImGui::SFML::ProcessEvent(e);
#endif
        if (e.type == sf::Event::Closed) {
            mWindow.close();
        }

        if (!mLocalInputEnabled) {
            return {};
        }

        if (e.type == sf::Event::MouseButtonPressed) {
            if (e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);
                InputObject input{};
                input.mType = ActionType::PRESS;
                input.action.mTarget = mousePosition;

                return input;
            }
        }
    }
    return {};
}

void InputDispatcher::disableLocalInput() { mLocalInputEnabled = false; }
void InputDispatcher::enableLocalInput() { mLocalInputEnabled = true; }
bool InputDispatcher::isLocalInputEnabled() const { return mLocalInputEnabled; }