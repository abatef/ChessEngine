#ifndef _BOARD_H_
#define _BOARD_H_

#include <vector>

#include "square.h"
class Board {
   public:
    using BoardT = std::vector<std::vector<Square::SquarePtr>>;
    Board() = default;
    bool init();
    BoardT &getSquares();

   private:
    BoardT squares;
};

#endif