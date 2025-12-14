#include "move_utility.h"
#include <cstdint>
#include <iostream>
#include <vector>

namespace {

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square)                                              \
  (get_bit(bitboard, square) ? (bitboard ^= (1ULL << square)) : 0)

uint8_t count_bits(uint64_t bitboard) {
  uint8_t count = 0;

  while (bitboard) {
    count++;
    bitboard &= bitboard - 1;
  }

  return count;
}

uint8_t get_ls1b_index(uint64_t bitboard) {
  if (bitboard != 0) {
    return count_bits((bitboard & (~bitboard + 1)) - 1);
  } else {
    return 64;
  }
}

uint64_t set_occupancy(uint16_t index, uint8_t bits_in_mask,
                       uint64_t attack_mask) {
  uint64_t occupancy = 0ULL;

  for (uint8_t count = 0; count < bits_in_mask; count++) {
    uint8_t square = get_ls1b_index(attack_mask);
    pop_bit(attack_mask, square);
    if (index & (1 << count))
      occupancy |= (1ULL << square);
  }

  return occupancy;
}

uint64_t generate_rook_attacks_rays(uint8_t square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;
  uint8_t target_rank = square / 8;
  uint8_t target_file = square % 8;
  int rank, file;

  // Right
  for (rank = target_rank + 1; rank < 8; rank++) {
    attacks |= (1ULL << (rank * 8 + target_file));
    if ((1ULL << (rank * 8 + target_file)) & occupancy)
      break;
  }
  // Left
  for (rank = target_rank - 1; rank >= 0; rank--) {
    attacks |= (1ULL << (rank * 8 + target_file));
    if ((1ULL << (rank * 8 + target_file)) & occupancy)
      break;
  }
  // Up
  for (file = target_file + 1; file < 8; file++) {
    attacks |= (1ULL << (target_rank * 8 + file));
    if ((1ULL << (target_rank * 8 + file)) & occupancy)
      break;
  }
  // Down
  for (file = target_file - 1; file >= 0; file--) {
    attacks |= (1ULL << (target_rank * 8 + file));
    if ((1ULL << (target_rank * 8 + file)) & occupancy)
      break;
  }

  return attacks;
}

uint64_t generate_bishop_attacks_rays(uint8_t square, uint64_t occupancy) {
  uint64_t attacks = 0ULL;
  uint8_t target_rank = square / 8;
  uint8_t target_file = square % 8;
  int rank, file;

  // NorthEast
  for (rank = target_rank + 1, file = target_file + 1; rank < 8 && file < 8;
       rank++, file++) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }
  // NorthWest
  for (rank = target_rank + 1, file = target_file - 1; rank < 8 && file > -1;
       rank++, file--) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }
  // SouthWest
  for (rank = target_rank - 1, file = target_file - 1; rank > -1 && file > -1;
       rank--, file--) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }
  // SouthEast
  for (rank = target_rank - 1, file = target_file + 1; rank > -1 && file < 8;
       rank--, file++) {
    attacks |= (1ULL << (rank * 8 + file));
    if ((1ULL << (rank * 8 + file)) & occupancy)
      break;
  }

  return attacks;
}

const std::array<uint64_t, 64> ROOK_MAGICS = {
    612498416294952992ULL,  2377936612260610304ULL,  36037730568766080ULL,
    72075188908654856ULL,   144119655536003584ULL,   5836666216720237568ULL,
    9403535813175676288ULL, 1765412295174865024ULL,  3476919663777054752ULL,
    288300746238222339ULL,  9288811671472386ULL,     146648600474026240ULL,
    3799946587537536ULL,    704237264700928ULL,      10133167915730964ULL,
    2305983769267405952ULL, 9223634270415749248ULL,  10344480540467205ULL,
    9376496898355021824ULL, 2323998695235782656ULL,  9241527722809755650ULL,
    189159985010188292ULL,  2310421375767019786ULL,  4647717014536733827ULL,
    5585659813035147264ULL, 1442911135872321664ULL,  140814801969667ULL,
    1188959108457300100ULL, 288815318485696640ULL,   758869733499076736ULL,
    234750139167147013ULL,  2305924931420225604ULL,  9403727128727390345ULL,
    9223970239903959360ULL, 309094713112139074ULL,   38290492990967808ULL,
    3461016597114651648ULL, 181289678366835712ULL,   4927518981226496513ULL,
    1155212901905072225ULL, 36099167912755202ULL,    9024792514543648ULL,
    4611826894462124048ULL, 291045264466247688ULL,   83880127713378308ULL,
    1688867174481936ULL,    563516973121544ULL,      9227888831703941123ULL,
    703691741225216ULL,     45203259517829248ULL,    693563138976596032ULL,
    4038638777286134272ULL, 865817582546978176ULL,   13835621555058516608ULL,
    11541041685463296ULL,   288511853443695360ULL,   283749161902275ULL,
    176489098445378ULL,     2306124759338845321ULL,  720584805193941061ULL,
    4977040710267061250ULL, 10097633331715778562ULL, 325666550235288577ULL,
    1100057149646ULL,
};

const std::array<uint64_t, 64> BISHOP_MAGICS = {
    9368648609924554880ULL, 9009475591934976ULL,     4504776450605056ULL,
    1130334595844096ULL,    1725202480235520ULL,     288516396277699584ULL,
    613618303369805920ULL,  10168455467108368ULL,    9046920051966080ULL,
    36031066926022914ULL,   1152925941509587232ULL,  9301886096196101ULL,
    290536121828773904ULL,  5260205533369993472ULL,  7512287909098426400ULL,
    153141218749450240ULL,  9241386469758076456ULL,  5352528174448640064ULL,
    2310346668982272096ULL, 1154049638051909890ULL,  282645627930625ULL,
    2306405976892514304ULL, 11534281888680707074ULL, 72339630111982113ULL,
    8149474640617539202ULL, 2459884588819024896ULL,  11675583734899409218ULL,
    1196543596102144ULL,    5774635144585216ULL,     145242600416216065ULL,
    2522607328671633440ULL, 145278609400071184ULL,   5101802674455216ULL,
    650979603259904ULL,     9511646410653040801ULL,  1153493285013424640ULL,
    18016048314974752ULL,   4688397299729694976ULL,  9226754220791842050ULL,
    4611969694574863363ULL, 145532532652773378ULL,   5265289125480634376ULL,
    288239448330604544ULL,  2395019802642432ULL,     14555704381721968898ULL,
    2324459974457168384ULL, 23652833739932677ULL,    282583111844497ULL,
    4629880776036450560ULL, 5188716322066279440ULL,  146367151686549765ULL,
    1153170821083299856ULL, 2315697107408912522ULL,  2342448293961403408ULL,
    2309255902098161920ULL, 469501395595331584ULL,   4615626809856761874ULL,
    576601773662552642ULL,  621501155230386208ULL,   13835058055890469376ULL,
    3748138521932726784ULL, 9223517207018883457ULL,  9237736128969216257ULL,
    1127068154855556ULL,
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

const std::array<uint32_t, 64> ROOK_BUFFER = {
  0,4096,6144,8192,10240,12288,14336,16384,
  20480,22528,23552,24576,25600,26624,27648,28672,
  30720,32768,33792,34816,35840,36864,37888,38912,
  40960,43008,44032,45056,46080,47104,48128,49152,
  51200,53248,54272,55296,56320,57344,58368,59392,
  61440,63488,64512,65536,66560,67584,68608,69632,
  71680,73728,74752,75776,76800,77824,78848,79872,
  81920,86016,88064,90112,92160,94208,96256,98304,
};

const std::array<uint32_t, 64> BISHOP_BUFFER = {
  0,64,96,128,160,192,224,256,
  320,352,384,416,448,480,512,544,
  576,608,640,768,896,1024,1152,1184,
  1216,1248,1280,1408,1920,2432,2560,2592,
  2624,2656,2688,2816,3328,3840,3968,4000,
  4032,4064,4096,4224,4352,4480,4608,4640,
  4672,4704,4736,4768,4800,4832,4864,4896,
  4928,4992,5024,5056,5088,5120,5152,5184,
};

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

// First index: side to move: WHITE (0) or BLACK (1)
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
          // WEST
          pawn_attacks[0][idx] += 1ULL << (idx + 7);
          pawn_attacks[1][idx] += 1ULL << (idx - 9);
        }
        if (i < 7) {
          // EAST
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

std::array<uint32_t, 64> init_rook_buffer(){
  std::array<uint32_t, 64> rook_buffer;
  uint32_t sum = 0;
  for (int i = 0; i < 64; i++) {
    rook_buffer[i] = sum;
    uint32_t buffer = 1ULL << ROOK_RELEVANT_BITS[i];
    sum += buffer;
    if (i % 8 == 0)
      std::cout << std::endl;
    std::cout << rook_buffer[i] << ",";
  }
  std::cout << std::endl;
  return rook_buffer;
}

std::array<uint32_t, 64> init_bishop_buffer(){
  std::array<uint32_t, 64> bishop_buffer;
  uint32_t sum = 0;
  for (int i = 0; i < 64; i++) {
    bishop_buffer[i] = sum;
    uint32_t buffer = 1ULL << BISHOP_RELEVANT_BITS[i];
    sum += buffer;
    if (i % 8 == 0)
      std::cout << std::endl;
    std::cout << bishop_buffer[i] << ",";
  }
  std::cout << std::endl;
  return bishop_buffer;
}

std::array<uint64_t, 102400> init_rook_table() {
  std::array<uint64_t, 102400> rook_table;
  uint32_t table_idx = 0;
  
  for (uint16_t square = 0; square < 64; square++) {
    uint8_t relevant_bits = 64 - MoveUtility::ROOK_MAGIC_ENTRY[square].shift;
    uint16_t table_size = 1ULL << relevant_bits;
    std::vector<uint64_t> attacks(table_size);
    // std::vector<uint64_t> hashed_attacks(table_size);
    std::vector<uint64_t> occupancies(table_size);
    uint64_t magic = MoveUtility::ROOK_MAGIC_ENTRY[square].magic;
    uint64_t mask = MoveUtility::ROOK_MAGIC_ENTRY[square].mask;

    // Initialize occupancy permutations and respective attacks
    for (uint16_t count = 0; count < table_size; count++) {
      occupancies[count] = set_occupancy(count, relevant_bits,mask);
      attacks[count] = generate_rook_attacks_rays(square, occupancies[count]);
    }

    // Assign attack tables to corresponding magic indexed hash table
    for (uint16_t count = 0; count < table_size; count++) {
      uint16_t magic_index = (occupancies[count] * magic) >> (64 - relevant_bits);
      table_idx = MoveUtility::ROOK_MAGIC_ENTRY[square].offset + magic_index;
      rook_table[table_idx] = attacks[count];
    }
  }
  return rook_table;
}

std::array<uint64_t, 5248> init_bishop_table() {
  std::array<uint64_t, 5248> bishop_table;
  uint32_t table_idx = 0;
  
  for (uint16_t square = 0; square < 64; square++) {
    uint8_t relevant_bits = 64 - MoveUtility::BISHOP_MAGIC_ENTRY[square].shift;
    uint16_t table_size = 1ULL << relevant_bits;
    std::vector<uint64_t> attacks(table_size);
    std::vector<uint64_t> occupancies(table_size);
    uint64_t magic = MoveUtility::BISHOP_MAGIC_ENTRY[square].magic;
    uint64_t mask = MoveUtility::BISHOP_MAGIC_ENTRY[square].mask;

    // Initialize occupancy permutations and respective attacks
    for (uint16_t count = 0; count < table_size; count++) {
      occupancies[count] = set_occupancy(count, relevant_bits,mask);
      attacks[count] = generate_bishop_attacks_rays(square, occupancies[count]);
    }

    // Assign attack tables to corresponding magic indexed hash table
    for (uint16_t count = 0; count < table_size; count++) {
      uint16_t magic_index = (occupancies[count] * magic) >> (64 - relevant_bits);
      table_idx = MoveUtility::BISHOP_MAGIC_ENTRY[square].offset + magic_index;
      bishop_table[table_idx] = attacks[count];
    }
  }
  return bishop_table;
}

std::array<MoveUtility::MagicEntry, 64> init_rook_magic_entry() {
  std::array<MoveUtility::MagicEntry, 64> rook_magic_entry;
  std::array<uint64_t, 64> rook_masks = init_rook_mask_table();
  for (int i = 0; i < 64; i++) {
    rook_magic_entry[i].magic = ROOK_MAGICS[i];
    rook_magic_entry[i].mask = rook_masks[i];
    rook_magic_entry[i].offset = ROOK_BUFFER[i];
    rook_magic_entry[i].shift = 64 - ROOK_RELEVANT_BITS[i];
  }
  return rook_magic_entry;
}

std::array<MoveUtility::MagicEntry, 64> init_bishop_magic_entry() {
  std::array<MoveUtility::MagicEntry, 64> bishop_magic_entry;
  std::array<uint64_t, 64> bishop_masks = init_bishop_mask_table();
  for (int i = 0; i < 64; i++) {
    bishop_magic_entry[i].magic = BISHOP_MAGICS[i];
    bishop_magic_entry[i].mask = bishop_masks[i];
    bishop_magic_entry[i].offset = BISHOP_BUFFER[i];
    bishop_magic_entry[i].shift = 64 - BISHOP_RELEVANT_BITS[i];
  }
  return bishop_magic_entry;
}

}



namespace MoveUtility {

const std::array<uint64_t, 64> KNIGHT_MOVES = init_knight_table();
const std::array<uint64_t, 64> KING_MOVES = init_king_table();
const std::array<std::array<uint64_t, 64>, 2> PAWN_ATTACKS = init_pawn_attack_table();
const std::array<uint8_t, 64> CASTLING_RIGHTS_UPDATE = init_c_rights_update();

const std::array<MagicEntry, 64> ROOK_MAGIC_ENTRY = init_rook_magic_entry();
const std::array<MagicEntry, 64> BISHOP_MAGIC_ENTRY = init_bishop_magic_entry();
const std::array<uint64_t, 102400> ROOK_ATTACKS = init_rook_table();
const std::array<uint64_t, 5248> BISHOP_ATTACKS = init_bishop_table();

} // namespace MoveUtility