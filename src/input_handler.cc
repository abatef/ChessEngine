#include "input_handler.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <iostream>

InputDispatcher::InputDispatcher(sf::RenderWindow& window) : m_Window(window) {}

InputObject InputDispatcher::captureInput() {
    sf::Event e;
    while (m_Window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            m_Window.close();
        }

        if (e.type == sf::Event::MouseButtonPressed) {
            if (e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(m_Window);
                InputObject input{};
                input.type = ActionType::PRESS;
                input.action.target = mousePosition;

                return input;
            }
        }
    }
    return {};
}