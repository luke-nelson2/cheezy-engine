#pragma once
#include <algorithm>
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
  Position(std::string fen_string) {

    uint8_t piece_str_len = fen_string.find(' ');
    std::string piece_str = fen_string.substr(0,piece_str_len);
    char side_char = fen_string[piece_str_len+1];
    uint8_t castle_idx = piece_str_len + 3;
    uint8_t ep_idx = fen_string.find(' ', castle_idx) + 1;
    uint8_t hm_idx = fen_string.find(' ', ep_idx) + 1;
    uint8_t fm_idx = fen_string.find(' ', hm_idx) + 1;

    std::string castle_string = fen_string.substr(castle_idx, ep_idx - castle_idx - 1);
    std::string ep_string = fen_string.substr(ep_idx, hm_idx - ep_idx - 1);
    std::string hm_string = fen_string.substr(hm_idx, fm_idx - hm_idx - 1);
    std::string fm_string = fen_string.substr(fm_idx);

    set_pieces(piece_str);
    set_castling(castle_string);
    set_ep(ep_string);
    
    halfmove_clock = std::stoi(hm_string);
    fullmove_count = std::stoi(fm_string);
    
    if (side_char == 'w') {
      side_to_move = WHITE;
    } else {
      side_to_move = BLACK;
    }
  }

  Position() {
    all_piece_bitboards[WHITE_PAWN] = 0xFF00ULL;
    all_piece_bitboards[WHITE_KNIGHT] = 0x42ULL;
    all_piece_bitboards[WHITE_BISHOP] = 0x24ULL;
    all_piece_bitboards[WHITE_ROOK] = 0x81ULL;
    all_piece_bitboards[WHITE_KING] = 0x10ULL;
    all_piece_bitboards[WHITE_QUEEN] = 0x8ULL;

    all_piece_bitboards[BLACK_PAWN] = 0xFF'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_KNIGHT] = 0x42'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_BISHOP] = 0x24'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_ROOK] = 0x81'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_KING] = 0x10'00'00'00'00'00'00'00ULL;
    all_piece_bitboards[BLACK_QUEEN] = 0x8'00'00'00'00'00'00'00ULL;

    side_to_move = 0;
    ply = 0;
    castling_rights = 0xF;
    en_passant_sq = 64;

    // PAWNS = 0/0b0    0
    // KNIGHT = 1/0b1   2
    // BISHOP = 2/0b10  4
    // ROOK = 3/0b11    6
    // QUEEN = 4/0b100  8
    // KING = 5/0b101   10
    // ADD ONE IF BLACK

    occupancy_bitboards[0] = 0;
    occupancy_bitboards[1] = 0;
    total_bb = 0;

    for (int i = 0; i<=10; i+=2){
      occupancy_bitboards[0] |= all_piece_bitboards[i];
      occupancy_bitboards[1] |= all_piece_bitboards[i+1];
    }

    total_bb = occupancy_bitboards[0] | occupancy_bitboards[1];


    // Set up piece lists
    for (int i = 0; i<64; i++) {
      piece_list[i] = NO_PIECE;
    }

    // PAWNS 
    for (int i = 8; i<16; i++) {
      // WHITE 0b00
      piece_list[i] = WHITE_PAWN;
      // BLACK 0b00
      piece_list[i+40] = BLACK_PAWN;
    }

    // WHITE KNIGHTS 0b10
    piece_list[1] = WHITE_KNIGHT;
    piece_list[6] = WHITE_KNIGHT;

    // BLACK KNIGHTS 0b11
    piece_list[57] = BLACK_KNIGHT;
    piece_list[62] = BLACK_KNIGHT;

    // WHITE BISHOPS
    piece_list[2] = WHITE_BISHOP;
    piece_list[5] = WHITE_BISHOP;

    // BLACK BISHOPS
    piece_list[58] = BLACK_BISHOP;
    piece_list[61] = BLACK_BISHOP;

    // WHITE ROOKS
    piece_list[0] = WHITE_ROOK;
    piece_list[7] = WHITE_ROOK;

    // BLACK ROOKS
    piece_list[56] = BLACK_ROOK;
    piece_list[63] = BLACK_ROOK;

    // WHITE QUEEN
    piece_list[3] = WHITE_QUEEN;

    // BLACK QUEEN
    piece_list[59] = BLACK_QUEEN;

    // WHITE KING
    piece_list[4] = WHITE_KING;

    // BLACK KING
    piece_list[60] = BLACK_KING;
  }

  void print_position() {
  for (int i = 7; i >= 0; i--) {
    for (int j = 0; j <= 7; j++) {
      uint8_t square = i * 8 + j;
      std::string square_str = ". ";

      if (piece_list[square] == BLACK_PAWN) square_str = "p ";
      if (piece_list[square] == BLACK_KNIGHT) square_str = "n ";
      if (piece_list[square] == BLACK_BISHOP) square_str = "b ";
      if (piece_list[square] == BLACK_ROOK) square_str = "r ";
      if (piece_list[square] == BLACK_QUEEN) square_str = "q ";
      if (piece_list[square] == BLACK_KING) square_str = "k ";

      if (piece_list[square] == WHITE_PAWN) square_str = "P ";
      if (piece_list[square] == WHITE_KNIGHT) square_str = "N ";
      if (piece_list[square] == WHITE_BISHOP) square_str = "B ";
      if (piece_list[square] == WHITE_ROOK) square_str = "R ";
      if (piece_list[square] == WHITE_QUEEN) square_str = "Q ";
      if (piece_list[square] == WHITE_KING) square_str = "K ";

      std::cout << square_str;
    }
    std::cout << std::endl;
  }
  std::cout << "\n";
}
  
  // Updates all relevant bitboards according to move
  // Assumes legal move
  void make_move(Move move){

    UndoInfo& move_record = history_stack[ply];
    uint8_t from_sq = move.get_from_sq();
    uint8_t to_sq = move.get_to_sq();
    uint8_t flags = move.get_flags();
    uint8_t moving_piece_type = piece_list[from_sq];
    uint8_t captured_piece_type = piece_list[to_sq];
    uint64_t from_bit = 1ULL << from_sq;
    uint64_t to_bit = 1ULL << to_sq;
    uint64_t move_mask = from_bit | to_bit;

    // UPDATE:
    // moving PIECE BITBOARD
    // CAPTURED PIECE BITBOARD (IF APPLICABLE)
    // OCCUPANCY BITBOARD

    history_stack[ply].castling_rights = castling_rights;
    history_stack[ply].move = move;
    history_stack[ply].captured_piece_type = captured_piece_type;
    history_stack[ply].en_passant_sq = en_passant_sq;
    history_stack[ply].halfmove_clock = halfmove_clock;

    // Move moving piece
    all_piece_bitboards[moving_piece_type] ^= move_mask;

    // Update moving color board
    occupancy_bitboards[moving_piece_type & 1] ^= move_mask;

    // Remove Captured Piece
    if (captured_piece_type < NO_PIECE) {
      all_piece_bitboards[captured_piece_type] ^= to_bit;
      occupancy_bitboards[captured_piece_type & 1] ^= to_bit;
    }

    // Update Piece Lists
    piece_list[from_sq] = NO_PIECE;
    piece_list[to_sq] = moving_piece_type;

    // Update other board state variables
    castling_rights &= ~MoveUtility::CASTLING_RIGHTS_UPDATE[from_sq];
    castling_rights &= ~MoveUtility::CASTLING_RIGHTS_UPDATE[to_sq];

    en_passant_sq = 64;
    if ((moving_piece_type >> 1) == PAWN) {
      if (std::abs((int)to_sq - (int)from_sq) == 16) {
        en_passant_sq = (from_sq + to_sq) >> 1;
      } 
    }

    if (flags) {
      if (flags == CASTLE_KINGSIDE) {

        uint64_t rook_mask = (1ULL << (to_sq - 1)) | (1ULL << (to_sq + 1));

        all_piece_bitboards[WHITE_ROOK + side_to_move] ^= rook_mask;
        occupancy_bitboards[side_to_move] ^= rook_mask;

        piece_list[to_sq + 1] = NO_PIECE;
        piece_list[to_sq - 1] = WHITE_ROOK + side_to_move;

      } else if (flags == CASTLE_QUEENSIDE) {

        uint64_t rook_mask = (1ULL << (to_sq - 2)) | (1ULL << (to_sq + 1));

        all_piece_bitboards[WHITE_ROOK + side_to_move] ^= rook_mask;
        occupancy_bitboards[side_to_move] ^= rook_mask;

        piece_list[to_sq - 2] = NO_PIECE;
        piece_list[to_sq + 1] = WHITE_ROOK + side_to_move;

      } else if (flags == EN_PASSANT) {

        uint8_t captured_sq = to_sq - 8 + (side_to_move << 4);
        uint64_t captured_bb = (1ULL << captured_sq);

        all_piece_bitboards[BLACK_PAWN - side_to_move] ^= captured_bb;
        occupancy_bitboards[BLACK - side_to_move] ^= captured_bb;
        piece_list[captured_sq] = NO_PIECE;

      } else if (flags >= PROMO_KNIGHT && flags <= PROMO_QUEEN) {

        uint8_t promo_piece_type = (flags << 1) + side_to_move;

        all_piece_bitboards[moving_piece_type] ^= to_bit;
        all_piece_bitboards[promo_piece_type] ^= to_bit;

        piece_list[to_sq] = promo_piece_type;
      }
    }

    if ((moving_piece_type < WHITE_KNIGHT) || (captured_piece_type != NO_PIECE)) {
      halfmove_clock = 0;
    } else {
      halfmove_clock++;
    }

    total_bb = occupancy_bitboards[WHITE] | occupancy_bitboards[BLACK];
    side_to_move ^= 1;
    ply++;
  }

  void unmake_move() {

    const UndoInfo& move_record = history_stack[ply-1];
    uint8_t from_sq = move_record.move.get_from_sq();
    uint8_t to_sq = move_record.move.get_to_sq();
    uint8_t flags = move_record.move.get_flags();
    uint64_t from_bit = 1ULL << from_sq;
    uint64_t to_bit = 1ULL << to_sq;
    uint64_t move_mask = from_bit | to_bit;

    uint8_t moving_piece_type = piece_list[to_sq];

    side_to_move ^= 1;

    castling_rights = move_record.castling_rights;
    en_passant_sq = move_record.en_passant_sq;

    // Move moving piece
    all_piece_bitboards[moving_piece_type] ^= move_mask;

    // Update moving color board
    occupancy_bitboards[moving_piece_type & 1] ^= move_mask;

    // Restore captured piece
    if (move_record.captured_piece_type < NO_PIECE) {
      all_piece_bitboards[move_record.captured_piece_type] ^= to_bit;
      occupancy_bitboards[move_record.captured_piece_type & 1] ^= to_bit;
    }

    // Update piece lists;
    piece_list[from_sq] = moving_piece_type;
    piece_list[to_sq] = move_record.captured_piece_type;

    // position.side_to_move is currently the opposite of whoever made the current move object
    // Handle flags
    if (flags) {
      if (flags == CASTLE_KINGSIDE) {

        uint64_t rook_mask = (1ULL << (to_sq - 1)) | (1ULL << (to_sq + 1));

        all_piece_bitboards[WHITE_ROOK + side_to_move] ^= rook_mask;
        occupancy_bitboards[side_to_move] ^= rook_mask;

        piece_list[to_sq + 1] = WHITE_ROOK + side_to_move;
        piece_list[to_sq - 1] = NO_PIECE;

      } else if (flags == CASTLE_QUEENSIDE) {

        uint64_t rook_mask = (1ULL << (to_sq + 1)) | (1ULL << (to_sq - 2));

        all_piece_bitboards[WHITE_ROOK + side_to_move] ^= rook_mask;
        occupancy_bitboards[side_to_move] ^= rook_mask;

        piece_list[to_sq - 2] = WHITE_ROOK + side_to_move;
        piece_list[to_sq + 1] = NO_PIECE;

      } else if (flags >= PROMO_KNIGHT && flags <= PROMO_QUEEN) {

        all_piece_bitboards[moving_piece_type] ^= from_bit;
        all_piece_bitboards[WHITE_PAWN + side_to_move] |= from_bit;

        piece_list[from_sq] = WHITE_PAWN + side_to_move;

      } else if (flags == EN_PASSANT) {

        uint8_t captured_sq = en_passant_sq - 8 + (side_to_move << 4);
        uint64_t captured_bit = (1ULL << captured_sq);

        all_piece_bitboards[BLACK_PAWN - side_to_move] ^= captured_bit;
        occupancy_bitboards[BLACK - side_to_move] ^= captured_bit;

        piece_list[captured_sq] = BLACK_PAWN - side_to_move;

      }
    }

    halfmove_clock = move_record.halfmove_clock;
    total_bb = occupancy_bitboards[WHITE] | occupancy_bitboards[BLACK];

    ply--;
  }

private:
  void set_pieces(std::string piece_str) {

    all_piece_bitboards.fill(0);
    occupancy_bitboards.fill(0);
    total_bb = 0;

    piece_list.fill(NO_PIECE);

    uint8_t str_length = piece_str.size();
    uint8_t rank = 7;
    uint8_t file = 0;

    for (int i = 0; i < str_length; i++) {
      
      uint8_t incr = 1;
      char piece_char = piece_str[i];

      if (piece_char == '/') {
        rank--;
        file = 0;
        continue;
      }

      uint8_t square = rank * 8 + file;
      uint64_t square_bit = (1ULL << square);
      uint8_t piece_type = NO_PIECE;

      if (piece_char == 'p') piece_type = BLACK_PAWN;
      if (piece_char == 'n') piece_type = BLACK_KNIGHT;
      if (piece_char == 'b') piece_type = BLACK_BISHOP;
      if (piece_char == 'r') piece_type = BLACK_ROOK;
      if (piece_char == 'q') piece_type = BLACK_QUEEN;
      if (piece_char == 'k') piece_type = BLACK_KING;

      if (piece_char == 'P') piece_type = WHITE_PAWN;
      if (piece_char == 'N') piece_type = WHITE_KNIGHT;
      if (piece_char == 'B') piece_type = WHITE_BISHOP;
      if (piece_char == 'R') piece_type = WHITE_ROOK;
      if (piece_char == 'Q') piece_type = WHITE_QUEEN;
      if (piece_char == 'K') piece_type = WHITE_KING;

      if (piece_char >= '0' && piece_char <= '9') {
        incr = piece_char - '0';
      }

      if (piece_type != NO_PIECE) {
        all_piece_bitboards[piece_type] |= square_bit;
        occupancy_bitboards[piece_type & 1U] |= square_bit;
        piece_list[square] = piece_type;
      }

      file += incr;
    }

    total_bb = occupancy_bitboards[WHITE] | occupancy_bitboards[BLACK];
  }

  void set_castling(std::string castle_string) {

    castling_rights = 0U;
    uint8_t str_len = castle_string.size();

    for (int i = 0; i < str_len; i++) {
      if (castle_string[i] == 'K') castling_rights |= 1U;
      if (castle_string[i] == 'Q') castling_rights |= 2U;
      if (castle_string[i] == 'k') castling_rights |= 4U;
      if (castle_string[i] == 'q') castling_rights |= 8U;
    }

  }

  void set_ep(std::string ep_string) {

    en_passant_sq = MoveUtility::NO_SQUARE;

    if (ep_string[0] != '-') {

      uint8_t file = ep_string[0] - 'a';
      uint8_t rank = ep_string[1] - '0';
      en_passant_sq = rank*8 + file;

    }

  }

};