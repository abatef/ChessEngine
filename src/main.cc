#include <iostream>
#include <memory>

#include "engine.h"
#include "pawn.h"
#include "piece.h"
#include "square.h"

int main() {
    Engine engine{};

    engine.loop();
}
