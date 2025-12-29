#include "move_generator.h"
#include "move.h"
#include "move_utility.h"
#include "piece.h"
#include "position.h"
#include <cstdint>
#include "search.h"

using namespace MoveUtility;

void MoveGenerator::generate(const Position& pos, const std::array<Move, 2> killers, const PST& hist_heur) {
  count = 0;
  ply_killers[0] = killers[0];
  ply_killers[1] = killers[1];
  history_heuristic = &hist_heur;

  if (pos.side_to_move == WHITE) {
    generate_all_moves<WHITE>(pos);
  } else {
    generate_all_moves<BLACK>(pos);
  }
}

inline void MoveGenerator::add_move(Move move, uint8_t moving_piece_type, uint8_t captured_piece_type) {

  move_list[count] = move;
  uint8_t flags = move.get_flags();

  if (flags >= PROMO_KNIGHT && flags <= PROMO_QUEEN) {
    if (flags == PROMO_QUEEN) {
      score_list[count] = QUEEN_PROMO_BONUS;
    } else {
      score_list[count] = WINNING_CAPTURE - 500'000;
    }
    if (captured_piece_type != NO_PIECE) {
      uint8_t victim_rank = PIECE_RANKS[captured_piece_type];
      score_list[count] += (10 * victim_rank);
    }
    count++;
    return;
  }

  if (captured_piece_type != NO_PIECE) {
    uint8_t attacker_rank = PIECE_RANKS[moving_piece_type];
    uint8_t victim_rank = PIECE_RANKS[captured_piece_type];

    int32_t mvv_lva_score = 10*(victim_rank) - attacker_rank;

    if (move.get_flags() == PROMO_QUEEN) {
      score_list[count] = QUEEN_PROMO_BONUS + mvv_lva_score;
      count++;
      return;
    }

    if (victim_rank > attacker_rank) {
      score_list[count] = WINNING_CAPTURE + mvv_lva_score;
    } else if (victim_rank == attacker_rank) {
      score_list[count] = EQUAL_CAPTURE + mvv_lva_score;
    } else {
      score_list[count] = LOSING_CAPTURE + mvv_lva_score;
    }
  } else {
    // QUIET MOVE
    // Killer move

    if (move == ply_killers[0]) {
      score_list[count] = KILLER_1_BAND;
    } else if (move == ply_killers[1]) {
      score_list[count] = KILLER_2_BAND;
    } else if (flags == CASTLE_KINGSIDE || flags == CASTLE_QUEENSIDE) {
      score_list[count] = CASTLE_BONUS;
    } else {
      // History Heuristic
      score_list[count] = (*history_heuristic)[moving_piece_type][move.get_to_sq()];
    }

  }

  count++;
}

bool MoveGenerator::is_square_attacked(const Position& pos, uint8_t square, uint8_t Us) {
  uint8_t Them = (Us == WHITE) ? BLACK : WHITE;

  // Pawn attack
  if (PAWN_ATTACKS[Us][square] & pos.all_piece_bitboards[WHITE_PAWN + Them]) return true;

  // Knight attack
  if (KNIGHT_MOVES[square] & pos.all_piece_bitboards[WHITE_KNIGHT + Them]) return true;

  // King attack
  if (KING_MOVES[square] & pos.all_piece_bitboards[WHITE_KING + Them]) return true;

  if (get_rook_attacks(square, pos.total_bb) &
    (pos.all_piece_bitboards[WHITE_ROOK + Them] | pos.all_piece_bitboards[WHITE_QUEEN + Them])) return true;

  if (get_bishop_attacks(square, pos.total_bb) &
    (pos.all_piece_bitboards[WHITE_BISHOP + Them] | pos.all_piece_bitboards[WHITE_QUEEN + Them])) return true;

  return false;
}

template<uint8_t Us>
void MoveGenerator::generate_all_moves(const Position& pos) {
  generate_pawn_moves<Us>(pos);
  generate_knight_moves<Us>(pos);
  generate_bishop_moves<Us>(pos);
  generate_rook_moves<Us>(pos);
  generate_queen_moves<Us>(pos);
  generate_king_moves<Us>(pos);
}

// "Us" is the side to move
template<uint8_t Us>
void MoveGenerator::generate_knight_moves(const Position& pos) {

  constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
  constexpr uint8_t moving_piece_type = (Us == WHITE) ? WHITE_KNIGHT : BLACK_KNIGHT;
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
      uint8_t to_piece_type = pos.piece_list[to_sq];
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, to_piece_type);
    }
  }
}

template<uint8_t Us>
void MoveGenerator::generate_bishop_moves(const Position& pos) {

  constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
  constexpr uint8_t moving_piece_type = (Us == WHITE) ? WHITE_BISHOP : BLACK_BISHOP;
  uint64_t temp_bishop_bb = pos.all_piece_bitboards[WHITE_BISHOP + Us];

  while (temp_bishop_bb) {

    uint8_t from_sq = get_lsbit_index(temp_bishop_bb);
    pop_bit(temp_bishop_bb, from_sq);
    uint64_t attacks = get_bishop_attacks(from_sq, pos.total_bb);
    attacks &= ~pos.occupancy_bitboards[WHITE + Us];

    while (attacks) {

      uint8_t to_sq = get_lsbit_index(attacks);
      pop_bit(attacks, to_sq);
      uint8_t to_piece_type = pos.piece_list[to_sq];
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, to_piece_type);
    }
  }
}

template<uint8_t Us>
void MoveGenerator::generate_rook_moves(const Position& pos) {

  constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
  constexpr uint8_t moving_piece_type = (Us == WHITE) ? WHITE_ROOK : BLACK_ROOK;
  uint64_t temp_rook_bb = pos.all_piece_bitboards[WHITE_ROOK + Us];

  while (temp_rook_bb) {

    uint8_t from_sq = get_lsbit_index(temp_rook_bb);
    pop_bit(temp_rook_bb, from_sq);
    uint64_t attacks = get_rook_attacks(from_sq, pos.total_bb);
    attacks &= ~pos.occupancy_bitboards[WHITE + Us];

    while (attacks) {

      uint8_t to_sq = get_lsbit_index(attacks);
      pop_bit(attacks, to_sq);
      uint8_t to_piece_type = pos.piece_list[to_sq];
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, to_piece_type);
    }
  }
}

template<uint8_t Us>
void MoveGenerator::generate_queen_moves(const Position& pos) {

  constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
  constexpr uint8_t moving_piece_type = (Us == WHITE) ? WHITE_QUEEN : BLACK_QUEEN;
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
      uint8_t to_piece_type = pos.piece_list[to_sq];
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, to_piece_type);
    }
  }
}

template<uint8_t Us>
void MoveGenerator::generate_king_moves(const Position& pos) {

  constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
  constexpr uint8_t moving_piece_type = (Us == WHITE) ? WHITE_KING : BLACK_KING;
  uint64_t temp_king_bb = pos.all_piece_bitboards[WHITE_KING + Us];

  while (temp_king_bb) {

    uint8_t from_sq = get_lsbit_index(temp_king_bb);
    pop_bit(temp_king_bb, from_sq);
    uint64_t attacks = KING_MOVES[from_sq];
    attacks &= ~pos.occupancy_bitboards[WHITE + Us];

    while (attacks) {

      uint8_t to_sq = get_lsbit_index(attacks);
      pop_bit(attacks, to_sq);
      uint8_t to_piece_type = pos.piece_list[to_sq];
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, to_piece_type);
    }
  }

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
      uint8_t from_sq = king_square;
      uint8_t to_sq = king_square + 2;
      Move move(from_sq, to_sq, CASTLE_KINGSIDE);
      add_move(move, moving_piece_type, NO_PIECE);
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
      uint8_t from_sq = king_square;
      uint8_t to_sq = king_square - 2;
      Move move(from_sq, to_sq, CASTLE_QUEENSIDE);
      add_move(move, moving_piece_type, NO_PIECE);
    }
  }

}

template<uint8_t Us>
void MoveGenerator::generate_pawn_moves(const Position& pos) {

  constexpr uint8_t Them = (Us == WHITE) ? BLACK : WHITE;
  constexpr uint8_t moving_piece_type = (Us == WHITE) ? WHITE_PAWN : BLACK_PAWN;
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
      Move queen_promo(from_sq, to_sq,PROMO_QUEEN);
      Move knight_promo(from_sq, to_sq,PROMO_KNIGHT);
      Move rook_promo(from_sq, to_sq,PROMO_ROOK);
      Move bishop_promo(from_sq, to_sq,PROMO_BISHOP);

      add_move(queen_promo, moving_piece_type, NO_PIECE);
      add_move(knight_promo, moving_piece_type, NO_PIECE);
      add_move(rook_promo, moving_piece_type, NO_PIECE);
      add_move(bishop_promo, moving_piece_type, NO_PIECE);
    } else {
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, NO_PIECE);
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

    Move move(from_sq, to_sq);
    add_move(move, moving_piece_type, NO_PIECE);
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
      Move queen_promo(from_sq, to_sq,PROMO_QUEEN);
      Move knight_promo(from_sq, to_sq,PROMO_KNIGHT);
      Move rook_promo(from_sq, to_sq,PROMO_ROOK);
      Move bishop_promo(from_sq, to_sq,PROMO_BISHOP);

      add_move(queen_promo, moving_piece_type, to_piece_type);
      add_move(knight_promo, moving_piece_type, to_piece_type);
      add_move(rook_promo, moving_piece_type, to_piece_type);
      add_move(bishop_promo, moving_piece_type, to_piece_type);
    } else {
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, to_piece_type);
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
      Move queen_promo(from_sq, to_sq,PROMO_QUEEN);
      Move knight_promo(from_sq, to_sq,PROMO_KNIGHT);
      Move rook_promo(from_sq, to_sq,PROMO_ROOK);
      Move bishop_promo(from_sq, to_sq,PROMO_BISHOP);

      add_move(queen_promo, moving_piece_type, to_piece_type);
      add_move(knight_promo, moving_piece_type, to_piece_type);
      add_move(rook_promo, moving_piece_type, to_piece_type);
      add_move(bishop_promo, moving_piece_type, to_piece_type);
    } else {
      Move move(from_sq, to_sq);
      add_move(move, moving_piece_type, to_piece_type);
    }
  }

  // En Passant
  if (pos.en_passant_sq != NO_SQUARE) {
    uint64_t ep_capturing = PAWN_ATTACKS[Them][pos.en_passant_sq];
    ep_capturing &= pawns;

    while (ep_capturing) {
      uint8_t from_sq = get_lsbit_index(ep_capturing);
      pop_bit(ep_capturing, from_sq);
      Move move(from_sq, pos.en_passant_sq, EN_PASSANT);
      add_move(move, moving_piece_type, WHITE_PAWN);
    }
  }
}
