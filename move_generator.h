#pragma once
// #include "move.h"
// #include "move_utility.h"
#include "move_utility.h"
#include "piece.h"
#include "position.h"
#include <cstdint>
#include <iostream>

using namespace MoveUtility;

// Move generator outputs a list of possible moves based on current board state
class MoveGenerator {
public:

  std::array<Move, 256> move_list;
  std::array<int32_t, 256> score_list;
  static constexpr std::array<uint8_t, 6> PIECE_SCORE{1,2,3,4,5,6};
  static const int32_t WINNING_CAPTURE = 6'000'000;
  static const int32_t EQUAL_CAPTURE = 5'000'000;
  static const int32_t LOSING_CAPTURE = -1'000'000;
  static const int32_t CASTLE_BONUS = 10'000;
  static const int32_t QUEEN_PROMO_BONUS = 7'000'000;
  uint8_t count;

  MoveGenerator() : count(0) {}

  void generate(const Position& pos) {
    count = 0;
    if (pos.side_to_move == WHITE) {
      generate_all_moves<WHITE>(pos);
    } else {
      generate_all_moves<BLACK>(pos);
    }
  }

  bool is_square_attacked(const Position& pos, uint8_t square, uint8_t Us) {
    uint8_t Them = (Us == WHITE) ? BLACK : WHITE;

    uint64_t bishop_attack = get_bishop_attacks(square, pos.total_bb) & 
      (pos.all_piece_bitboards[WHITE_BISHOP + Them] | pos.all_piece_bitboards[WHITE_QUEEN + Them]);

    uint64_t rook_attack = get_rook_attacks(square, pos.total_bb) & 
      (pos.all_piece_bitboards[WHITE_ROOK + Them] | pos.all_piece_bitboards[WHITE_QUEEN + Them]);

    // Knight attack
    uint64_t knight_attack = KNIGHT_MOVES[square] & pos.all_piece_bitboards[WHITE_KNIGHT + Them];

    // Pawn attack
    uint64_t pawn_attack = PAWN_ATTACKS[Us][square] & pos.all_piece_bitboards[WHITE_PAWN + Them];

    // King attack
    uint64_t king_attack = KING_MOVES[square] & pos.all_piece_bitboards[WHITE_KING + Them]; 

    if (bishop_attack | rook_attack | knight_attack | pawn_attack | king_attack) {
      return true;
    } else {
      return false;
    }
  }

private:

  template<uint8_t Us>
  void generate_all_moves(const Position& pos) {
    generate_pawn_moves<Us>(pos);
    generate_knight_moves<Us>(pos);
    generate_bishop_moves<Us>(pos);
    generate_rook_moves<Us>(pos);
    generate_queen_moves<Us>(pos);
    generate_king_moves<Us>(pos);
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

        uint8_t to_piece_type = pos.piece_list[to_sq];

        if (to_piece_type != NO_PIECE) {

          int32_t mvv_lva_score = 10*PIECE_SCORE[to_piece_type >> 1] - 2;

          if (to_piece_type >= WHITE_ROOK) score_list[count] = WINNING_CAPTURE + mvv_lva_score;
          else if (to_piece_type >= WHITE_KNIGHT) score_list[count] = EQUAL_CAPTURE + mvv_lva_score;
          else score_list[count] = LOSING_CAPTURE + mvv_lva_score;

        } else {
          score_list[count] = 0;
        }
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

        uint8_t to_piece_type = pos.piece_list[to_sq];
        if (to_piece_type != NO_PIECE) {

          int32_t mvv_lva_score = 10*PIECE_SCORE[to_piece_type >> 1] - 3;
          if (to_piece_type >= WHITE_ROOK) score_list[count] = WINNING_CAPTURE + mvv_lva_score;
          else if (to_piece_type >= WHITE_KNIGHT) score_list[count] = EQUAL_CAPTURE + mvv_lva_score;
          else score_list[count] = LOSING_CAPTURE + mvv_lva_score;

        } else {
          score_list[count] = 0;
        }
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

        uint8_t to_piece_type = pos.piece_list[to_sq];
        if (to_piece_type != NO_PIECE) {

          int32_t mvv_lva_score = 10*PIECE_SCORE[to_piece_type >> 1] - 4;
          if (to_piece_type >= WHITE_QUEEN) score_list[count] = mvv_lva_score + WINNING_CAPTURE;
          else if (to_piece_type >= WHITE_ROOK) score_list[count] = mvv_lva_score + EQUAL_CAPTURE;
          else score_list[count] = mvv_lva_score + LOSING_CAPTURE;

        } else {
          score_list[count] = 0;
        }

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
      uint64_t attacks = get_rook_attacks(from_sq, pos.total_bb) |
                         get_bishop_attacks(from_sq, pos.total_bb);
      attacks &= ~pos.occupancy_bitboards[WHITE + Us];
      while (attacks) {
        uint8_t to_sq = get_lsbit_index(attacks);
        pop_bit(attacks, to_sq);
        move_list[count] = Move(from_sq, to_sq);

        uint8_t to_piece_type = pos.piece_list[to_sq];
        if (to_piece_type != NO_PIECE) {

          int32_t mvv_lva_score = 10*((to_piece_type >> 1) + 1) - 5;
          if (to_piece_type >= WHITE_QUEEN) score_list[count] = mvv_lva_score + EQUAL_CAPTURE;
          else score_list[count] = mvv_lva_score + LOSING_CAPTURE;

        } else {
          score_list[count] = 0;
        }
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

        uint8_t to_piece_type = pos.piece_list[to_sq];
        if (to_piece_type != NO_PIECE) {
          score_list[count] = 10*((to_piece_type >> 1) + 1) - 6 + WINNING_CAPTURE;
        } else {
          score_list[count] = 0;
        }

        count++;
      }
    }

    // Castle moves
    // Check the following:
    // Castling rights flags
    // No pieces in between
    // Not in check
    // Does not pass through or finish on a square that is attacked

    // Kingside Castle
    const uint8_t kingside_castle = (Us == WHITE) ? pos.castling_rights & 1U : pos.castling_rights & 4U;
    const uint8_t queenside_castle = (Us == WHITE) ? pos.castling_rights & 2U : pos.castling_rights & 8U;
    constexpr uint64_t kingside_mask = (Us == WHITE) ? 0x60ULL : 0x60'00'00'00'00'00'00'00ULL;
    constexpr uint64_t queenside_mask = (Us == WHITE) ? 0xEULL : 0x0E'00'00'00'00'00'00'00ULL;
    constexpr uint8_t king_square = (Us == WHITE) ? 4U : 60U;
    bool fail;

    if (kingside_castle && !(pos.total_bb & kingside_mask)) {
      fail = false;
      for (uint8_t square = king_square; square < (king_square + 3); square++) {
        if (is_square_attacked(pos, square, Us)) {
          fail = true;
          break;
        }
      }

      if (!fail) {
        move_list[count] = Move(king_square, king_square + 2, CASTLE_KINGSIDE);
        score_list[count] = CASTLE_BONUS;
        count++;
      }
    }

    // Queenside Castle
    if (queenside_castle && !(pos.total_bb & queenside_mask)) {
      fail = false;
      for (uint8_t square = king_square; square > king_square - 3; square--) {
        if (is_square_attacked(pos, square, Us)) {
          fail = true;
          break;
        }
      }

      if (!fail) {
        move_list[count] = Move(king_square, king_square - 2, CASTLE_QUEENSIDE);
        score_list[count] = CASTLE_BONUS;
        count++;
      }
    }

  }

  template<uint8_t Us>
  void generate_pawn_moves(const Position& pos) {

    constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
    constexpr uint64_t PromotionRank = (Us == WHITE) ? RANK_8 : RANK_1;
    constexpr uint64_t DoublePushRank = (Us == WHITE) ? RANK_4 : RANK_5;
    constexpr uint8_t shift = (Us == WHITE) ? 8 : -8;

    uint64_t pawns = pos.all_piece_bitboards[WHITE_PAWN + Us];
    uint64_t enemies = pos.occupancy_bitboards[Them];

    // Single Push
    uint64_t single_pushes = 0ULL;
    if constexpr (Us == WHITE) {
      single_pushes = pawns << 8;
    } else {
      single_pushes = pawns >> 8;
    }

    single_pushes &= ~pos.total_bb;
    uint64_t push_loop = single_pushes;
    
    while(push_loop) {
      uint8_t to_sq = get_lsbit_index(push_loop);
      pop_bit(push_loop, to_sq);
      uint8_t from_sq = to_sq - shift;

      // Check promotion
      if (1ULL << to_sq & PromotionRank) {
        move_list[count] = Move(from_sq, to_sq,PROMO_QUEEN);
        move_list[count+1] = Move(from_sq, to_sq,PROMO_KNIGHT);
        move_list[count+2] = Move(from_sq, to_sq,PROMO_ROOK);
        move_list[count+3] = Move(from_sq, to_sq,PROMO_BISHOP);

        score_list[count] = QUEEN_PROMO_BONUS;
        for (int i = 1; i < 4; i++) {
          score_list[count+i] = 0;
        }
        count+=4;
      } else {
        move_list[count] = Move(from_sq, to_sq);
        score_list[count] = 0;
        count++;
      }
    }

    // Double Push
    uint64_t double_pushes = 0ULL;

    if constexpr (Us == WHITE) {
      double_pushes = single_pushes << 8;
    } else {
      double_pushes = single_pushes >> 8;
    }

    double_pushes &= DoublePushRank;
    double_pushes &= ~pos.total_bb;

    while(double_pushes) {
      uint8_t to_sq = get_lsbit_index(double_pushes);
      pop_bit(double_pushes, to_sq);
      uint8_t from_sq = to_sq - shift*2;

      move_list[count] = Move(from_sq, to_sq);
      score_list[count] = 0;
      count++;
    }

    // Capture
    uint64_t capture_right = (Us == WHITE) ? ((pawns & ~FILE_H) << 9) :
                                             ((pawns & ~FILE_A) >> 9);
    capture_right &= enemies;

    while (capture_right) {
      uint8_t to_sq = get_lsbit_index(capture_right);
      pop_bit(capture_right, to_sq);
      uint8_t from_sq = (Us == WHITE) ? (to_sq - 9) : (to_sq + 9);
      uint8_t to_piece_type = pos.piece_list[to_sq];

      // Check promotion
      if (1ULL << to_sq & PromotionRank) {
        move_list[count] = Move(from_sq, to_sq,PROMO_QUEEN);
        move_list[count+1] = Move(from_sq, to_sq,PROMO_KNIGHT);
        move_list[count+2] = Move(from_sq, to_sq,PROMO_ROOK);
        move_list[count+3] = Move(from_sq, to_sq,PROMO_BISHOP);

        int32_t mvv_lva_score = 10*((to_piece_type >> 1) + 1) - 1;
        score_list[count] = QUEEN_PROMO_BONUS + mvv_lva_score;
        for (int i = 1; i < 4; i++) {
          score_list[count+i] = WINNING_CAPTURE + mvv_lva_score;
        }
        count+=4;

      } else {
        move_list[count] = Move(from_sq, to_sq);
        int32_t mvv_lva_score = 10*((to_piece_type >> 1) + 1) - 1;
        if (to_piece_type <= BLACK_PAWN) score_list[count] = mvv_lva_score + EQUAL_CAPTURE;
        else score_list[count] = mvv_lva_score + WINNING_CAPTURE;
        count++;
      }
    }

    uint64_t capture_left = (Us == WHITE) ? ((pawns & ~FILE_A) << 7) :
                                             ((pawns & ~FILE_H) >> 7);
    capture_left &= enemies;

    while (capture_left) {
      uint8_t to_sq = get_lsbit_index(capture_left);
      uint8_t to_piece_type = pos.piece_list[to_sq];
      pop_bit(capture_left, to_sq);
      uint8_t from_sq = (Us == WHITE) ? (to_sq - 7) : (to_sq + 7);

      // Check promotion
      if (1ULL << to_sq & PromotionRank) {
        move_list[count] = Move(from_sq, to_sq,PROMO_QUEEN);
        move_list[count+1] = Move(from_sq, to_sq,PROMO_KNIGHT);
        move_list[count+2] = Move(from_sq, to_sq,PROMO_ROOK);
        move_list[count+3] = Move(from_sq, to_sq,PROMO_BISHOP);

        int32_t mvv_lva_score = 10*((to_piece_type >> 1) + 1) - 1;
        score_list[count] = QUEEN_PROMO_BONUS + mvv_lva_score;
        for (int i = 1; i < 4; i++) {
          score_list[count+i] = WINNING_CAPTURE + mvv_lva_score;
        }
        count+=4;

      } else {
        move_list[count] = Move(from_sq, to_sq);
        int32_t mvv_lva_score = 10*((to_piece_type >> 1) + 1) - 1;
        if (to_piece_type <= BLACK_PAWN) score_list[count] = mvv_lva_score + EQUAL_CAPTURE;
        else score_list[count] = mvv_lva_score + WINNING_CAPTURE;
        count++;
      }
    }

    // En Passant
    if (pos.en_passant_sq != NO_SQUARE) {
      uint64_t ep_capturing = PAWN_ATTACKS[Them][pos.en_passant_sq];
      ep_capturing &= pawns;

      while (ep_capturing) {
        uint8_t from_sq = get_lsbit_index(ep_capturing);
        pop_bit(ep_capturing, from_sq);
        move_list[count] = Move(from_sq, pos.en_passant_sq, EN_PASSANT);
        int32_t mvv_lva_score = 9;
        score_list[count] = mvv_lva_score + EQUAL_CAPTURE;
        count++;
      }
    }
  }
};