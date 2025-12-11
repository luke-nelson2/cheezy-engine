#pragma once
#include <cstdint>

struct Move {
  uint16_t move_data;

  Move() : move_data(0) {};

  Move(uint8_t from_sq, uint8_t to_sq, uint8_t flags=0) {
    move_data = (uint16_t)from_sq | ((uint16_t)to_sq << 6) | ((uint16_t)flags << 12);
  }
  
  inline uint8_t get_from_sq() const {
    return move_data & 0x3F;
  }

  inline uint8_t get_to_sq() const {
    return (move_data >> 6) & 0x3F;
  }

  inline uint8_t get_flags() const {
    return (move_data >> 12) & 0xF;
  }

  // FLAGS
  //   0 - NORMAL MOVE
  //   1 - PROMOTION
  //   2 - CASTLING
  //   3 - EN PASSANT
  //   PIECES TO PROMOTE TO: KNIGHT, BISHOP, ROOK, QUEEN
};