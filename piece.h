#include <cstdint>

enum Piece : uint8_t {
  WHITE = 0,
  BLACK = 1,

  WHITE_PAWN = 0,
  BLACK_PAWN = 1,
  WHITE_KNIGHT = 2,
  BLACK_KNIGHT = 3,
  WHITE_BISHOP = 4,
  BLACK_BISHOP = 5,
  WHITE_ROOK = 6,
  BLACK_ROOK = 7,
  WHITE_QUEEN = 8,
  BLACK_QUEEN = 9,
  WHITE_KING = 10,
  BLACK_KING = 11,

  PAWN = 0,
  KNIGHT = 1,
  BISHOP = 2,
  ROOK = 3,
  QUEEN = 4,
  KING = 5,

  NO_PIECE = 12
};