#pragma once
#include <array>
#include <cstdint>
#include <cmath>
#include "piece.h"
#include "move.h"
#include "move_utility.h"
#include <iostream>

struct UndoInfo {
  Move move;
  uint8_t captured_piece_type;
  uint8_t castling_rights; // 4 bits: white: king and queen side, black: king and queen side
  uint8_t en_passant_sq;
  uint8_t halfmove_clock;
  // uint8_t zobrist_key;
};

class Position{
public:
  std::array<uint64_t, 12> all_piece_bitboards;
  std::array<uint64_t, 2> occupancy_bitboards;
  uint64_t total_bb;

  uint8_t castling_rights;
  uint8_t en_passant_sq;
  bool side_to_move;
  // If halfmove_clock reaches 100, and side_to_move has at least 1 legal move,
  // Draw score assigned to that node
  uint8_t halfmove_clock;
  uint16_t fullmove_count;
  uint16_t ply;

  std::array<UndoInfo, 2048> history_stack;
  std::array<uint8_t, 64> piece_list;

  // FEN constructor
  Position(std::string fen_string);

  Position();

  void print_position();
  
  // Updates all relevant bitboards according to move
  // Assumes legal move
  void make_move(Move move);

  void unmake_move();

private:
  void set_pieces(std::string piece_str);

  void set_castling(std::string castle_string);

  void set_ep(std::string ep_string);

};