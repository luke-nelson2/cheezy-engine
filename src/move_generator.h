#pragma once
#include "position.h"

class MoveGenerator {

public:

  std::array<Move, 256> move_list;
  std::array<int32_t, 256> score_list;
  static const int32_t WINNING_CAPTURE = 6'000'000;
  static const int32_t EQUAL_CAPTURE = 5'000'000;
  static const int32_t LOSING_CAPTURE = -1'000'000;
  static const int32_t CASTLE_BONUS = 10'000;
  static const int32_t QUEEN_PROMO_BONUS = 7'000'000;
  int count;

  MoveGenerator() : count(0) {}

  void generate(const Position& pos);
  bool is_square_attacked(const Position& pos, uint8_t square, uint8_t Us);

private:

  template<uint8_t Us>
  void generate_all_moves(const Position& pos);

  template<uint8_t Us>
  void generate_knight_moves(const Position& pos);

  template<uint8_t Us>
  void generate_bishop_moves(const Position& pos);

  template<uint8_t Us>
  void generate_rook_moves(const Position& pos);

  template<uint8_t Us>
  void generate_queen_moves(const Position& pos);

  template<uint8_t Us>
  void generate_king_moves(const Position& pos);

  template<uint8_t Us>
  void generate_pawn_moves(const Position& pos);
  
};