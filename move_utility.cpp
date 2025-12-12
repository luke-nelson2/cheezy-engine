#include "move_utility.h"
#include <cstdint>
#include <iostream>

namespace MoveUtility {
const std::array<uint64_t, 64> ROOK_MAGICS = {
    0x8A80104000800020, 0x140002000100040,  0x2801880A0017001,
    0x100081001000420,  0x200020010080420,  0x3001C0002010008,
    0x8480008002000100, 0x2080088004402900, 0x800098204000,
    0x2024401000200040, 0x100802000801000,  0x120800800801000,
    0x208808088000400,  0x2802200800400,    0x2200800100020080,
    0x801000060821100,  0x80044006422000,   0x100808020004000,
    0x12108A0010204200, 0x140848010000802,  0x481828014002800,
    0x8094004002004100, 0x4010040010010802, 0x20008806104,
    0x100400080208000,  0x2040002120081000, 0x21200680100081,
    0x20100080080080,   0x2000A00200410,    0x20080800400,
    0x80088400100102,   0x80004600042881,   0x4040008040800020,
    0x440003000200801,  0x4200011004500,    0x188020010100100,
    0x14800401802800,   0x2080040080800200, 0x124080204001001,
    0x200046502000484,  0x480400080088020,  0x1000422010034000,
    0x30200100110040,   0x100021010009,     0x2002080100110004,
    0x202008004008002,  0x20020004010100,   0x2048440040820001,
    0x101002200408200,  0x40802000401080,   0x4008142004410100,
    0x2060820C0120200,  0x1001004080100,    0x20C020080040080,
    0x2935610830022400, 0x44440041009200,   0x280001040802101,
    0x2100190040002085, 0x80C0084100102001, 0x4024081001000421,
    0x20030A0244872,    0x12001008414402,   0x2006104900A0804,
    0x1004081002402,
};

const std::array<uint64_t, 64> BISHOP_MAGICS = {
    0x40040822862081,   0x40810A4108000,    0x2008008400920040,
    0x61050104000008,   0x8282021010016100, 0x41008210400A0001,
    0x3004202104050C0,  0x22010108410402,   0x60400862888605,
    0x6311401040228,    0x80801082000,      0x802A082080240100,
    0x1860061210016800, 0x401016010A810,    0x1000060545201005,
    0x21000C2098280819, 0x2020004242020200, 0x4102100490040101,
    0x114012208001500,  0x108000682004460,  0x7809000490401000,
    0x420B001601052912, 0x408C8206100300,   0x2231001041180110,
    0x8010102008A02100, 0x204201004080084,  0x410500058008811,
    0x480A040008010820, 0x2194082044002002, 0x2008A20001004200,
    0x40908041041004,   0x881002200540404,  0x4001082002082101,
    0x8110408880880,    0x8000404040080200, 0x200020082180080,
    0x1184440400114100, 0xC220008020110412, 0x4088084040090100,
    0x8822104100121080, 0x100111884008200A, 0x2844040288820200,
    0x90901088003010,   0x1000A218000400,   0x1102010420204,
    0x8414A3483000200,  0x6410849901420400, 0x201080200901040,
    0x204880808050002,  0x1001008201210000, 0x16A6300A890040A,
    0x8049000441108600, 0x2212002060410044, 0x100086308020020,
    0x484241408020421,  0x105084028429C085, 0x4282480801080C,
    0x81C098488088240,  0x1400000090480820, 0x4444000030208810,
    0x1020142010820200, 0x2234802004018200, 0xC2040450820A00,
    0x2101021090020,
};

const std::array<uint8_t, 64> ROOK_RELEVANT_BITS = {
    12, 11, 11, 11, 11, 11, 11, 12, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11, 12, 11, 11, 11, 11, 11, 11, 12};

// bishop relevant occupancy bits
const std::array<uint8_t, 64> BISHOP_RELEVANT_BITS = {
    6, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 7, 7, 7, 7,
    5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 9, 9, 7, 5, 5, 5, 5, 7, 7,
    7, 7, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 5, 5, 5, 5, 5, 5, 6};

std::array<uint64_t, 64> init_knight_table() {
  int idx;
  std::array<uint64_t, 64> knight_attacks;

  for (int rank = 0; rank < 8; rank++) {
    for (int file = 0; file < 8; file++) {
      uint64_t mask{0x0};
      idx = file * 8 + rank;
      uint64_t piece_bit = 1ULL << idx;
      uint64_t target;

      // SSW
      if (rank > 0 && file > 1) {
        target = piece_bit >> 17;
        mask |= target;
      }

      // SSE
      if (rank < 7 && file > 1) {
        target = piece_bit >> 15;
        mask |= target;
      }

      // ESE
      if (rank < 6 && file > 0) {
        target = piece_bit >> 6;
        mask |= target;
      }

      // ENE
      if (rank < 6 && file < 7) {
        target = piece_bit << 10;
        mask |= target;
      }

      // NNE
      if (rank < 7 && file < 6) {
        target = piece_bit << 17;
        mask |= target;
      }

      // NNW
      if (rank > 0 && file < 6) {
        target = piece_bit << 15;
        mask |= target;
      }

      // WNW
      if (rank > 1 && file < 7) {
        target = piece_bit << 6;
        mask |= target;
      }

      // WSW
      if (rank > 1 && file > 0) {
        target = piece_bit >> 10;
        mask |= target;
      }

      knight_attacks[idx] = mask;
    }
  }
  return knight_attacks;
}

std::array<std::array<uint64_t, 64>, 2> init_pawn_attack_table() {
  int idx;
  std::array<std::array<uint64_t, 64>, 2> pawn_attacks;
  for (int j = 0; j < 8; j++) {
    for (int i = 0; i < 8; i++) {
      idx = j * 8 + i;
      pawn_attacks[0][idx] = 0;
      pawn_attacks[1][idx] = 0;
      if (j > 0 && j < 7) {
        if (i > 0) {
          pawn_attacks[0][idx] += 1ULL << (idx + 7);
          pawn_attacks[1][idx] += 1ULL << (idx - 9);
        }
        if (i < 7) {
          pawn_attacks[0][idx] += 1ULL << (idx + 9);
          pawn_attacks[1][idx] += 1ULL << (idx - 7);
        }
      }
    }
  }
  return pawn_attacks;
}

std::array<uint64_t, 64> init_king_table() {
  int idx;
  std::array<uint64_t, 64> king_attacks;
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 8; j++) {
      idx = j * 8 + i;
      king_attacks[idx] = 0;
      if (i > 0) {
        // WEST
        king_attacks[idx] += (1ULL << (idx - 1));
        if (j > 0) {
          // SOUTH WEST
          king_attacks[idx] += (1ULL << (idx - 9));
        }
        if (j < 7) {
          // NORTH WEST
          king_attacks[idx] += (1ULL << (idx + 7));
        }
      }
      if (i < 7) {
        // EAST
        king_attacks[idx] += (1ULL << (idx + 1));
        if (j > 0) {
          // SOUTH EAST
          king_attacks[idx] += (1ULL << (idx - 7));
        }
        if (j < 7) {
          // NORTH EAST
          king_attacks[idx] += (1ULL << (idx + 9));
        }
      }
      if (j > 0) {
        // SOUTH
        king_attacks[idx] += (1ULL << (idx - 8));
      }
      if (j < 7) {
        // NORTH
        king_attacks[idx] += (1ULL << (idx + 8));
      }
    }
  }
  return king_attacks;
}

std::array<uint8_t, 64> init_c_rights_update() {
  std::array<uint8_t, 64> c_rights_update = {0};
  // Bit Order: White Kingside, White Queenside, Black Kingside, Black Queenside
  c_rights_update[0] = 2;
  c_rights_update[4] = 3;
  c_rights_update[7] = 1;
  c_rights_update[56] = 8;
  c_rights_update[60] = 12;
  c_rights_update[63] = 4;
  return c_rights_update;
}

std::array<uint64_t, 64> init_rook_mask_table() {
  std::array<uint64_t, 64> rook_mask_table = {0};
  uint8_t rank = 0;
  uint8_t file = 0;

  for (uint8_t i = 0; i < 64; i++) {
    rank = i / 8;
    file = i % 8;
    for (uint8_t j = 1; j < 7; j++) {
      // Rank moves
      if (j != file) {
        rook_mask_table[i] |= 1ULL << (rank * 8 + j);
      }
      if (j != rank) {
        rook_mask_table[i] |= 1ULL << (file + j * 8);
      }
    }
  }

  return rook_mask_table;
}

std::array<uint64_t, 64> init_bishop_mask_table() {
  std::array<uint64_t, 64> bishop_mask_table = {0};
  uint8_t target_rank = 0;
  uint8_t target_file = 0;

  for (int i = 0; i < 64; i++) {
    target_rank = i / 8;
    target_file = i % 8;
    int rank = 0, file = 0;
    // NorthEast
    for (rank = target_rank + 1, file = target_file + 1; rank < 7 && file < 7;
         rank++, file++) {
      bishop_mask_table[i] |= (1ULL << (rank * 8 + file));
    }
    // NorthWest
    for (rank = target_rank + 1, file = target_file - 1; rank < 7 && file > 0;
         rank++, file--) {
      bishop_mask_table[i] |= (1ULL << (rank * 8 + file));
    }
    // SouthWest
    for (rank = target_rank - 1, file = target_file - 1; rank > 0 && file > 0;
         rank--, file--) {
      bishop_mask_table[i] |= (1ULL << (rank * 8 + file));
    }
    // SouthEast
    for (rank = target_rank - 1, file = target_file + 1; rank > 0 && file < 7;
         rank--, file++) {
      bishop_mask_table[i] |= (1ULL << (rank * 8 + file));
    }
  }

  return bishop_mask_table;
}

std::array<uint64_t, 102400> init_rook_table() {
  
}

const std::array<uint64_t, 102400> ROOK_ATTACKS = {0};
const std::array<uint64_t, 5248> BISHOP_ATTACKS = {0};
const std::array<uint64_t, 64> KNIGHT_MOVES = init_knight_table();
const std::array<uint64_t, 64> KING_MOVES = init_king_table();
const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS =
    init_pawn_attack_table();
const std::array<uint8_t, 64> CASTLING_RIGHTS_UPDATE = init_c_rights_update();
const std::array<uint64_t, 64> ROOK_MASK_TABLE = init_rook_mask_table();
const std::array<uint64_t, 64> BISHOP_MASK_TABLE = init_bishop_mask_table();
} // namespace MoveUtility

int main() {
  uint32_t sum = 0;
  for (int i = 0; i < 64; i++) {
    int x = 1ULL << MoveUtility::BISHOP_RELEVANT_BITS[i];
    sum += x;
  }
  std::cout << sum << std::endl;
  return 0;
}