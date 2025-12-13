#include "move_utility.h"
#include "position.h"
#include <cstdint>
#include <iostream>

using namespace MoveUtility;

// Move generator outputs a list of possible moves based on current board state
class MoveGenerator {
public:

  std::array<Move, 256> move_list;
  uint8_t count;

  void generate_moves(const Position& pos) {

  }

private:

  void generate_pawn_moves() {

  }

  // "Us" is the side to move
  template<uint8_t Us>
  void generate_knight_moves(const Position& pos) {

    constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
    // Find knight bb
    uint64_t temp_knight_bb = pos.all_piece_bitboards[WHITE_KNIGHT + Us];
    
    // While-pop iteration
    while(temp_knight_bb) {

      uint8_t from_sq = get_lsbit_index(temp_knight_bb);
      pop_bit(temp_knight_bb, from_sq);
      uint64_t attacks = KNIGHT_MOVES[from_sq];

      // Filter moves where the destination square contains a friendly piece
      attacks &= ~pos.occupancy_bitboards[WHITE + Us];

      while (attacks) {
        uint8_t to_sq = get_lsbit_index(attacks);
        pop_bit(attacks, to_sq);
        move_list[count] = Move(from_sq, to_sq);
        count++;
      }
    }
  }

  template<uint8_t Us>
  void generate_bishop_moves(const Position& pos) {

    constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
    uint64_t temp_bishop_bb = pos.all_piece_bitboards[WHITE_BISHOP + Us];

    while (temp_bishop_bb) {

      uint8_t from_sq = get_lsbit_index(temp_bishop_bb);
      pop_bit(temp_bishop_bb, from_sq);
      uint64_t attacks = get_bishop_attacks(from_sq, pos.total_bb);
      attacks &= ~pos.occupancy_bitboards[WHITE + Us];

      while (attacks) {

        uint8_t to_sq = get_lsbit_index(attacks);
        pop_bit(attacks, to_sq);
        move_list[count] = Move(from_sq, to_sq);
        count++;
      }
    }
  }

  template<uint8_t Us>
  void generate_rook_moves(const Position& pos) {

    constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
    uint64_t temp_rook_bb = pos.all_piece_bitboards[WHITE_ROOK + Us];

    while (temp_rook_bb) {

      uint8_t from_sq = get_lsbit_index(temp_rook_bb);
      pop_bit(temp_rook_bb, from_sq);
      uint64_t attacks = get_rook_attacks(from_sq, pos.total_bb);
      attacks &= ~pos.occupancy_bitboards[WHITE + Us];

      while (attacks) {

        uint8_t to_sq = get_lsbit_index(attacks);
        pop_bit(attacks, to_sq);
        move_list[count] = Move(from_sq, to_sq);
        count++;
      }
    }
  }

  template<uint8_t Us>
  void generate_queen_moves(const Position& pos) {

    constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
    uint64_t temp_queen_bb = pos.all_piece_bitboards[WHITE_QUEEN + Us];

    while (temp_queen_bb) {

      uint8_t from_sq = get_lsbit_index(temp_queen_bb);
      pop_bit(temp_queen_bb, from_sq);
      uint64_t attacks = get_rook_attacks(from_sq, pos.total_bb) &
                         get_bishop_attacks(from_sq, pos.total_bb);
      attacks &= ~pos.occupancy_bitboards[WHITE + Us];
      while (attacks) {
        uint8_t to_sq = get_lsbit_index(attacks);
        pop_bit(attacks, to_sq);
        move_list[count] = Move(from_sq, to_sq);
        count++;
      }
    }
  }

  template<uint8_t Us>
  void generate_king_moves(const Position& pos) {

    constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
    uint64_t temp_king_bb = pos.all_piece_bitboards[WHITE_KING + Us];

    while (temp_king_bb) {

      uint8_t from_sq = get_lsbit_index(temp_king_bb);
      pop_bit(temp_king_bb, from_sq);
      uint64_t attacks = KING_MOVES[from_sq];
      attacks &= ~pos.occupancy_bitboards[WHITE + Us];

      while (attacks) {
        
        uint8_t to_sq = get_lsbit_index(attacks);
        pop_bit(attacks, to_sq);
        move_list[count] = Move(from_sq, to_sq);
        count++;
      }
    }
  }
};