#pragma once
#include "piece.h"
#include "position.h"
#include <cstdint>
#include "search.h"
#include "move.h"

class MoveGenerator {

public:

  std::array<Move, 256> move_list;
  std::array<int32_t, 256> score_list;
  std::array<Move, 2> ply_killers;
  const PST* history_heuristic;
  Move current_pv_move;
  static const int32_t PV_BAND = 8'000'000;
  static const int32_t QUEEN_PROMO_BONUS = 7'000'000;
  static const int32_t WINNING_CAPTURE = 6'000'000;
  static const int32_t EQUAL_CAPTURE = 5'000'000;
  static const int32_t KILLER_1_BAND = 4'000'000;
  static const int32_t KILLER_2_BAND = 3'000'000;
  static const int32_t LOSING_CAPTURE = -1'000'000;
  static const int32_t CASTLE_BONUS = 10'000;
  
  int count;

  MoveGenerator() : count(0) {}

  void generate(const Position& pos, const std::array<Move, 2> killers, const PST& hist_heur, Move pv_move);
  bool is_square_attacked(const Position& pos, uint8_t square, uint8_t Us);

private:

  static constexpr std::array<uint8_t, 12> PIECE_RANKS = {1, 1, 2, 2, 2, 2, 3, 3, 4, 4, 5, 5};

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

  inline void add_move(Move move, uint8_t moving_piece_type, uint8_t captured_piece_type);

};
