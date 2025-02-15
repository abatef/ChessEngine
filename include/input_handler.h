#ifndef _INPUT_HANDLER_H_
#define _INPUT_HANDLER_H_

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Network/TcpListener.hpp>
#include <SFML/Network/TcpSocket.hpp>
#include <SFML/System/Vector2.hpp>
#include <cstdint>
#include <exception>
#include <iostream>
#include <string>

enum class ActionType { NONE, PRESS, ENGINE };

struct InputObject {
    ActionType type = ActionType::NONE;
    sf::Vector2i coord;
    std::string to_string() const {
        std::string r = "";
        if (type == ActionType::PRESS) {
            r += "PRESS: (" + std::to_string(coord.x) + ", " + std::to_string(coord.y) + ")";
        }
        return r;
    }
};

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

class InputDispatcherBase {
   private:
    bool m_isEnabled = true;

   public:
    bool isEnabled() const { return m_isEnabled; }
    void enable() { m_isEnabled = true; }
    void disable() { m_isEnabled = false; }
};

class MouseInputDispatcher : public InputDispatcherBase {
   private:
    sf::RenderWindow &m_window;

   public:
    MouseInputDispatcher() = delete;
    MouseInputDispatcher(sf::RenderWindow &window)
        : m_window(window) {}
    InputObject captureInput();
};

/*
    Data Layout
    +--------+------------+------------+
    | ACTION |   X COORD  |   Y COORD  |
    | 8 Bit  |   12 Bit   |   12 Bit   |
    +--------+------------+------------+
     0x00 => NONE
     0xf0 => PRESS
     0x0f => ENGINE
*/
#define ACTION_MASK (0xff << 24)
#define PRESS_ACTION (0xf0 << 24)
#define XCOORD_MASK (0xfff << 12)
#define YCOORD_MASK (0xfff)

struct InputEncoder {
    static uint32_t encode(InputObject &io) {
        uint32_t data = 0;
        if (io.type == ActionType::NONE) {
            return 0;
        }
        if (io.type == ActionType::PRESS) {
            data |= (0xf0) << 24;
            int x = io.coord.x;
            data |= (x << 12);
            int y = io.coord.y;
            data |= y;
        }
        return data;
    }
    static InputObject decode(uint32_t date) {
        InputObject io{};
        io.type = ActionType::NONE;
        if ((date & ACTION_MASK) == PRESS_ACTION) {
            io.type = ActionType::PRESS;
            int x = (date & XCOORD_MASK) >> 12;
            int y = date & YCOORD_MASK;
            io.coord.x = x;
            io.coord.y = y;
        }
        return io;
    }
};

class SocketInputDispatcher : public InputDispatcherBase {
   public:
    enum class SocketMode { HOST, CLIENT };
    class SocketException : public std::exception {
       private:
        std::string msg;

       public:
        SocketException(std::string msg_)
            : msg(msg_) {}
        const char *what() { return msg.c_str(); }
    };

   private:
    sf::TcpSocket m_Socket;
    sf::TcpListener m_Listener;
    bool m_Connected;
    bool m_isListening;
    SocketMode m_SocketMode;

   public:
    SocketInputDispatcher(SocketMode socketMode)
        : m_SocketMode(socketMode)
        , m_Connected(false)
        , m_isListening(false) {};
    ~SocketInputDispatcher() {
        m_Socket.disconnect();
        m_Listener.close();
    }
    void connect(std::string host, short port);
    bool isConnected() { return m_Connected; }
    InputObject receive();
    void send(InputObject &io);
};

class FullInputHandler {
   private:
    SocketInputDispatcher socket;
    MouseInputDispatcher mouse;

   public:
    FullInputHandler() = delete;
    FullInputHandler(SocketInputDispatcher::SocketMode socketMode, sf::RenderWindow &window)
        : socket(socketMode)
        , mouse(window) {}

    void disableMouse() { mouse.disable(); }
    void enableMouse() { mouse.enable(); }
    InputObject handleInput();
};

#endif