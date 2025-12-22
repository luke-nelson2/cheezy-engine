#include <cstdint>
#include "position.h"
#include "move.h"

int32_t negamax(Position& pos, uint8_t depth, int32_t alpha, int32_t beta);
Move negamax_root(Position& pos, uint8_t depth);