#include "engine.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>

#include "board.h"
#include "input_handler.h"
#include "piece.h"
#include "square.h"

Engine::Engine() : m_Handler(m_Renderer.getWindow()) { m_Board.init(); }

Engine::Engine(InputHandler handler) : m_Handler(m_Renderer.getWindow()) {}

const sf::Color GREEN_SQUARE(118, 150, 86);

void Engine::loop() {
    while (m_Renderer.isRunning()) {
        m_Handler.handleInput();
        m_Renderer.drawBoard(m_Board);
        m_Renderer.update();
        sf::sleep(sf::milliseconds(10));
    }
}