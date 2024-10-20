#include "engine.h"

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>
#include <iostream>
#include <ostream>

#include "board.h"
#include "input_handler.h"
#include "piece.h"
#include "renderer.h"

Engine::Engine() : m_Handler(m_Renderer.getWindow()) { m_Board.init(); }

Engine::Engine(InputDispatcher handler) : m_Handler(m_Renderer.getWindow()) {}

const sf::Color GREEN_SQUARE(118, 150, 86);

Square::SquarePtr Engine::selectedSquare = nullptr;

void Engine::handleInput() {
    InputObject io = m_Handler.captureInput();
    if (io.type != ActionType::NONE) {
        m_Renderer.m_DrawFlag = true;
    }
    if (io.type == ActionType::PRESS) {
        sf::Vector2i target = io.action.target;
        std::cout << "SELECT ON X: " << io.action.target.x << " Y: " << io.action.target.y
                  << std::endl;
        auto currentSquare = m_Board.selectSquare(target);
        if (selectedSquare == nullptr) {
            std::cout << "NULL SELECT" << std::endl;
            selectedSquare = currentSquare;
            currentSquare->select();
            return;
        }
        // the pieces move from the selected to the current not vice versa
        if (currentSquare != selectedSquare) {
            if (!selectedSquare->isOccupied()) {
                selectedSquare->deSelect();
                selectedSquare = currentSquare;
                currentSquare->select();
                return;
            }
            if (selectedSquare->isOccupied()) {
                if (!currentSquare->isOccupied()) {
                    auto occupier = selectedSquare->getOccupier();
                    selectedSquare->clear();
                    selectedSquare->deSelect();
                    occupier->deOccupy();
                    currentSquare->setOccupier(occupier);
                    occupier->setSquare(currentSquare);
                    selectedSquare = nullptr;
                } else {
                    if (currentSquare->getOccupier()->getColor() ==
                        selectedSquare->getOccupier()->getColor()) {
                        selectedSquare->deSelect();
                        selectedSquare = currentSquare;
                        currentSquare->select();
                    } else {
                        auto occupier = selectedSquare->getOccupier();
                        auto opponent = currentSquare->getOccupier();
                        currentSquare->clear();
                        selectedSquare->clear();
                        selectedSquare->deSelect();
                        currentSquare->setOccupier(occupier);
                        occupier->setSquare(currentSquare);
                        opponent->deOccupy();
                    }
                }
            }
            return;
        }
        if (currentSquare == selectedSquare) {
            currentSquare->deSelect();
            selectedSquare = nullptr;
            std::cout << "DESELECT" << std::endl;
            return;
        }
    }
}

void Engine::loop() {
    while (m_Renderer.isRunning()) {
        handleInput();
        m_Renderer.drawBoard(m_Board);
        m_Renderer.update();
        sf::sleep(sf::milliseconds(10));
    }
}