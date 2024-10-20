#include "input_handler.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>

InputHandler::InputHandler(sf::RenderWindow& window) : m_Window(window) {}

void InputHandler::handleInput() {
    sf::Event e;
    while (m_Window.pollEvent(e)) {
        if (e.type == sf::Event::Closed) {
            m_Window.close();
        }
    }
}