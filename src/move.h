#pragma once
#include <cstdint>

enum Flag : uint8_t {
  NORMAL_MOVE,
  PROMO_KNIGHT,
  PROMO_BISHOP,
  PROMO_ROOK,
  PROMO_QUEEN,
  CASTLE_KINGSIDE,
  CASTLE_QUEENSIDE,
  EN_PASSANT
};

struct Move {
  uint16_t move_data;

  Move() : move_data(0) {};

  Move(uint8_t from_sq, uint8_t to_sq, uint8_t flags=0) {
    move_data = (uint16_t)from_sq | ((uint16_t)to_sq << 6) | ((uint16_t)flags << 12);
  }

  inline bool operator== (const Move& other) const {
    return move_data == other.move_data;
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
};

  // FLAGS
  //   0 - NORMAL MOVE
  //   1-4 - PROMOTION
  //   1 - KNIGHT
  //   2 - BISHOP
  //   3 - ROOK
  //   4 - QUEEN
  //   5 - CASTLE KINGSIDE
  //   6 - CASTLE QUEENSIDE
  //   7 - EN PASSANT