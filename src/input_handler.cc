#include "input_handler.h"

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Mouse.hpp>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <ostream>

#include "SFML/Network/Socket.hpp"
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
                input.type = ActionType::PRESS;
                input.coord = mousePosition;

                return input;
            }
        }
    }
    return {};
}

void InputDispatcher::disableLocalInput() { mLocalInputEnabled = false; }
void InputDispatcher::enableLocalInput() { mLocalInputEnabled = true; }
bool InputDispatcher::isLocalInputEnabled() const { return mLocalInputEnabled; }

InputObject MouseInputDispatcher::captureInput() {
    sf::Event e;
    while (m_window.pollEvent(e)) {
#ifdef IMGUI_MODE
        ImGui::SFML::ProcessEvent(e);
#endif
        if (e.type == sf::Event::Closed) {
            m_window.close();
        }

        if (!isEnabled()) {
            return {};
        }

        if (e.type == sf::Event::MouseButtonPressed) {
            if (e.mouseButton.button == sf::Mouse::Left) {
                sf::Vector2i mousePosition = sf::Mouse::getPosition(m_window);
                InputObject input{};
                input.type = ActionType::PRESS;
                input.coord = mousePosition;

                return input;
            }
        }
    }
    return {};
}

void SocketInputDispatcher::connect(std::string host, short port) {
    if (m_Connected) return;
    m_Socket.setBlocking(false);
    if (m_SocketMode == SocketMode::CLIENT) {
        sf::Socket::Status status = m_Socket.connect(host, port);
        if (status == sf::Socket::Status::NotReady) {
            std::cout << "Connected to: " << host << ":" << port << std::endl;
            m_Connected = true;
        }
    } else if (m_SocketMode == SocketMode::HOST) {
        if (!m_isListening) {
            m_Listener.listen(port);
            std::cout << "Listening on port: " << port << std::endl;
            m_isListening = true;
            return;
        }
        if (m_Listener.accept(m_Socket) == sf::Socket::Done) {
            m_Connected = true;
            std::cout << "Client Connected" << std::endl;
        }
    }
}

void SocketInputDispatcher::send(InputObject& io) {
    if (!m_Connected) throw SocketException("socket is not connected");
    uint32_t data = InputEncoder::encode(io);
    std::size_t sent = 0, total = 0;
    int loops = 0;
    while (total < 4) {
        sf::Socket::Status status = m_Socket.send(&data, 4, sent);
        if (status == sf::Socket::Partial || status == sf::Socket::Done) {
            total += sent;
        }
    }
}

InputObject SocketInputDispatcher::receive() {
    if (!m_Connected) throw SocketException("socket is not connected");
    uint32_t data;
    std::size_t received = 0;
    std::size_t total = 4;
    while (total > received) {
        sf::Socket::Status status = m_Socket.receive(&data, 4, received);
    }
    return InputEncoder::decode(data);
}

InputObject FullInputHandler::handleInput() {
    InputObject io;
    return io;
}