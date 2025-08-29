#include <cassert>
#include <iostream>

#include "engine.h"

void test_HostMode() {
    SocketInputDispatcher sid(SocketInputDispatcher::SocketMode::HOST);
    while (!sid.isConnected()) {
        sid.connect("127.0.0.1", 9999);
    }
    InputObject io = sid.receive();
    std::cout << io.to_string() << std::endl;
}

void test_ClientMode() {
    SocketInputDispatcher sid(SocketInputDispatcher::SocketMode::CLIENT);
    while (!sid.isConnected()) {
        sid.connect("127.0.0.1", 9999);
    }
    InputObject io{};
    io.type = ActionType::PRESS;
    io.coord.x = 299;
    io.coord.y = 859;
    sid.send(io);
}

int main() {
    Engine engine{};

    engine.loop();
}
