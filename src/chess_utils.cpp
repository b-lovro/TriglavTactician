#include "./chess_utils.h"

// Mapping from square indices to their algebraic notation.
// clang-format off
const char *square_to_position[65] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", "no_sq"
};

// clang-format on

// Converts a character representing a piece to its corresponding enumeration value.
int charToPieceEnum(char pieceChar) {
  switch (pieceChar) {
    case 'P':
      return WP;
    case 'N':
      return WN;
    case 'B':
      return WB;
    case 'R':
      return WR;
    case 'Q':
      return WQ;
    case 'K':
      return WK;
    case 'p':
      return BP;
    case 'n':
      return BN;
    case 'b':
      return BB;
    case 'r':
      return BR;
    case 'q':
      return BQ;
    case 'k':
      return BK;
    default:
      return EMPTY;
  }
}

// Counts the number of set bits (1s) in a bitboard, using the most efficient method available.
int countBits(U64 bitboard) {
#ifdef __GNUC__  // GCC, Clang, or any GCC-compatible compiler
  return __builtin_popcountll(bitboard);

#elif defined(_MSC_VER)  // Microsoft Visual C++
  return static_cast<int>(__popcnt64(bitboard));

#else
  // Fallback to Brian Kernighan's algorithm for other compilers
  int count = 0;
  while (bitboard) {
    count++;
    bitboard &= bitboard - 1;  // Clear the least significant set bit
  }
  return count;

#endif
}

// Finds the index of the least significant set bit in a bitboard.
unsigned int bitScanForward(U64 bb) {
  assert(bb != 0);  // Ensure bb is not zero

#ifdef __GNUC__  // GCC, Clang, or any GCC-compatible compiler
  return __builtin_ctzll(bb);

#elif defined(_MSC_VER)  // Microsoft Visual C++
  unsigned long index;
  _BitScanForward64(&index, bb);
  return index;

#else
  // Fallback to De Bruijn Multiplication
  //@author Martin Läuter (1997), Charles E. Leiserson, Harald Prokop, Keith H. Randall
  const U64 debruijn64 = U64(0x03f79d71b4cb0a89);
  return INDEX_64_FORWARD[((bb & -bb) * debruijn64) >> 58];

#endif
}

// Finds the index of the most significant set bit in a bitboard.
unsigned int bitScanReverse(U64 bb) {
  assert(bb != 0);  // Ensure bb is not zero.

#ifdef __GNUC__  // GCC, Clang, or any GCC-compatible compiler
  return 63 - __builtin_clzll(bb);

#elif defined(_MSC_VER)  // Microsoft Visual C++
  unsigned long index;
  if (_BitScanReverse64(&index, bb)) {
    return index;
  } else {
    return -1;  // Adjust based on your needs.
  }

#else
  // Fallback to De Bruijn Multiplication
  // @authors Kim Walisch, Mark Dickinson
  bb |= bb >> 1;
  bb |= bb >> 2;
  bb |= bb >> 4;
  bb |= bb >> 8;
  bb |= bb >> 16;
  bb |= bb >> 32;
  const U64 debruijn64 = U64(0x03f79d71b4cb0a89);
  return INDEX_64_REVERSE[(bb * debruijn64) >> 58];

#endif
}

// =====================================
//        MOVE GENERATION LOGIC
// =====================================

// Initialize arrays used for move generation.
U64 rays[8][64] = {};
U64 pawn_attacks[2][64] = {};
U64 knight_attacks[64] = {};
U64 king_attacks[64] = {};

// --- SLIDING PIECES ---
// Possible moves generated on the fly,
// NOTE: improvement would be to use magic numbers

/**
 * Initializes the directional ray bitboards for all squares on the chessboard. This function
 * pre-computes and stores in global arrays the bitboard representations for each direction a
 * sliding piece (bishop, rook, queen) can move from any given square.
 *
 * Directions include up, down, left, right, and the four diagonals: up-left, up-right, down-left,
 * and down-right. The function handles edge cases to prevent ray wrapping from one side of the board
 * to the other.
 */
void initGenerateRays() {
  const U64 ONE = U64(1);

  // Loop over all squares of the chessboard
  for (int square = 0; square < 64; square++) {
    // Calculate the row and column for the current square
    int row = square / 8;
    int col = square % 8;

    // Initialize masks for diagonal movements
    U64 downLeftMask = 0x102040810204000ULL;
    U64 downRightMask = 0x8040201008040200ULL;
    U64 upLeftMask = 0x40201008040201ULL;
    U64 upRightMask = 0x2040810204080ULL;

    // Generate vertical and horizontal rays
    rays[DOWN][square] = 0x0101010101010100ULL << square;
    rays[UP][square] = 0x0080808080808080ULL >> (63 - square);
    rays[RIGHT][square] = 2 * ((ONE << (square | 7)) - (ONE << square));
    rays[LEFT][square] = (ONE << square) - (ONE << (square & 56));

    // Adjust masks for diagonal rays based on the current square's location
    // On each step: Shift the mask one square to the right/left and mask with
    // NOT_FILE_X to prevent wrapping around to the 'X' file.

    for (int i = 0; i < 7 - col; i++) {
      downLeftMask = (downLeftMask >> 1) & NOT_FILE_H;
    }
    // Shift the mask down to the correct row, placing the ray on the chessboard relative to the square.
    rays[DOWNLEFT][square] = downLeftMask << (row * 8);
    for (int i = 0; i < col; i++) {
      downRightMask = (downRightMask << 1) & NOT_FILE_A;
    }
    // -||-
    rays[DOWNRIGHT][square] = downRightMask << (row * 8);
    for (int i = 0; i < 7 - col; i++) {
      upLeftMask = (upLeftMask >> 1) & NOT_FILE_H;
    }
    // -||-
    rays[UPLEFT][square] = upLeftMask >> ((7 - row) * 8);
    for (int i = 0; i < col; i++) {
      upRightMask = (upRightMask << 1) & NOT_FILE_A;
    }
    // -||-
    rays[UPRIGHT][square] = upRightMask >> ((7 - row) * 8);
  }
}

/**
 * Generates all possible bishop moves from a given square, considering the current blockers on the board.
 * This function calculates bishop attacks by using pre-computed rays for the diagonal directions.
 *
 * @param square; The square index where the bishop is located.
 * @param blockers; Bitboard (U64) representing the positions of all pieces on the board that can block the bishop's
 * movement.
 * @return Bitboard (U64) representing all possible attack squares for the bishop.
 */
U64 getBishopMoves(unsigned int square, U64 blockers) {
  // Init empty attacks bitboard
  U64 attacks = 0ULL;

  // Diagonal attacks: Upleft, Upright, Downright, Downleft
  // Each direction is calculated by merging the ray in that direction with adjustments for any blockers.

  attacks |= rays[UPLEFT][square];
  if (rays[UPLEFT][square] & blockers) {
    int blockerIndex = bitScanReverse(rays[UPLEFT][square] & blockers);  // Find the first blocker
    attacks &= ~rays[UPLEFT][blockerIndex];                              // Remove possible moves beyond the blocker
  }

  attacks |= rays[UPRIGHT][square];
  if (rays[UPRIGHT][square] & blockers) {
    int blockerIndex = bitScanReverse(rays[UPRIGHT][square] & blockers);
    attacks &= ~rays[UPRIGHT][blockerIndex];
  }

  attacks |= rays[DOWNRIGHT][square];
  if (rays[DOWNRIGHT][square] & blockers) {
    int blockerIndex = bitScanForward(rays[DOWNRIGHT][square] & blockers);
    attacks &= ~rays[DOWNRIGHT][blockerIndex];
  }

  attacks |= rays[DOWNLEFT][square];
  if (rays[DOWNLEFT][square] & blockers) {
    int blockerIndex = bitScanForward(rays[DOWNLEFT][square] & blockers);
    attacks &= ~rays[DOWNLEFT][blockerIndex];
  }

  return attacks;
}

/**
 * Generates all possible rook moves from a given square, considering the current blockers on the board.
 * This function calculates rook attacks by using pre-computed rays for the vertical and horizontal directions.
 *
 * @param square; The square index where the rook is located.
 * @param blockers; Bitboard (U64) representing the positions of all pieces on the board that can block the rook's
 * movement.
 * @return Bitboard (U64) representing all possible attack squares for the rook.
 */
U64 getRooksMoves(unsigned int square, U64 blockers) {
  U64 attacks = 0ULL;

  // Vertical and horizontal attacks: Up, Down, Right, Left
  // Each direction is calculated by merging the ray in that direction with adjustments for any blockers.

  attacks |= rays[UP][square];
  if (rays[UP][square] & blockers) {
    int blockerIndex = bitScanReverse(rays[UP][square] & blockers); // Find the first blocker
    attacks &= ~rays[UP][blockerIndex];                             // Remove possible moves beyond the blocker
  }
  attacks |= rays[DOWN][square];
  if (rays[DOWN][square] & blockers) {
    int blockerIndex = bitScanForward(rays[DOWN][square] & blockers);
    attacks &= ~rays[DOWN][blockerIndex];
  }
  attacks |= rays[RIGHT][square];
  if (rays[RIGHT][square] & blockers) {
    int blockerIndex = bitScanForward(rays[RIGHT][square] & blockers);
    attacks &= ~rays[RIGHT][blockerIndex];
  }
  attacks |= rays[LEFT][square];
  if (rays[LEFT][square] & blockers) {
    int blockerIndex = bitScanReverse(rays[LEFT][square] & blockers);
    attacks &= ~rays[LEFT][blockerIndex];
  }
  return attacks;
}

/**
 * Generates all possible queen moves from a given square, considering the current blockers on the board.
 * This function combines the attack patterns of a rook and a bishop to reflect the queen's movement capabilities.
 *
 * @param square; The square index (0-63) where the queen is located.
 * @param blockers; Bitboard (U64) representing the positions of all pieces on the board that can block the queen's movement.
 * @return Bitboard (U64) representing all possible attack squares for the queen.
 */
U64 getQueensMoves(unsigned int square, U64 blockers) {
  return getRooksMoves(square, blockers) | getBishopMoves(square, blockers);
}

// --- LEAPER PIECES ---

/**
 * Generates the attack bitboard for a pawn at a given square and color. This function calculates
 * all the potential attacking moves a pawn can make from the specified square, considering its
 * color.
 *
 * @param color; (0 for white, 1 for black).
 * @param square; The square index of the pawn.
 * @return Bitboard representing all squares the pawn can attack from its current position.
 */
U64 generatePawnAttacks(unsigned int color, unsigned int square) {
  // Init attacks bitboard
  U64 attacks = 0ULL;

  // Initialize the pawn's position on an empty bitboard
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // Generate pawn attacks, considering board edges to avoid wrapping
  if (!color) {
    // White pawns
    if ((bitboard >> 7) & NOT_FILE_A) attacks |= (bitboard >> 7);
    if ((bitboard >> 9) & NOT_FILE_H) attacks |= (bitboard >> 9);
  } else {
    // Black pawns
    if ((bitboard << 7) & NOT_FILE_H) attacks |= (bitboard << 7);
    if ((bitboard << 9) & NOT_FILE_A) attacks |= (bitboard << 9);
  }

  // Return attack bitboard
  return attacks;
}

/**
 * Generates the attack bitboard for a king at a given square. This function calculates all the
 * potential moves a king can make.
 *
 * @param square; The square index of the king.
 * @return Bitboard representing all squares the king can move to (or attack) from its current position.
 */
U64 generateKingAttacks(unsigned int square) {
  // Init attacks bitboard
  U64 attacks = 0ULL;

  // Initialize the king's position on an empty bitboard
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // Generate king attacks, considering board edges to avoid wrapping
  // Moves are generated in all 8 possible directions from the king's square
  if (bitboard >> 8) attacks |= (bitboard >> 8);                 // Up
  if ((bitboard >> 9) & NOT_FILE_H) attacks |= (bitboard >> 9);  // Up-right
  if ((bitboard >> 7) & NOT_FILE_A) attacks |= (bitboard >> 7);  // Up-left
  if ((bitboard >> 1) & NOT_FILE_H) attacks |= (bitboard >> 1);  // Right
  if (bitboard << 8) attacks |= (bitboard << 8);                 // Down
  if ((bitboard << 9) & NOT_FILE_A) attacks |= (bitboard << 9);  // Down-left
  if ((bitboard << 7) & NOT_FILE_H) attacks |= (bitboard << 7);  // Down-right
  if ((bitboard << 1) & NOT_FILE_A) attacks |= (bitboard << 1);  // Left

  // Return attack bitboard
  return attacks;
}

/**
 * Generates the attack bitboard for a knight at a given square. This function calculates all
 * the potential moves (including attacks) a knight can make from the specified square.
 *
 * @param square; The square index of the knight.
 * @return Bitboard representing all squares the knight can move to (or attack) from its current position.
 */
U64 generateKnightAttacks(unsigned int square) {
  // Init attacks bitboard
  U64 attacks = 0ULL;

  // Initialize the knight's position on an empty bitboard
  U64 bitboard = 0ULL;
  set_bit(bitboard, square);

  // Generate knight attacks, considering board edges to avoid wrapping
  // Moves are generated for each of the 8 possible L-shapes from the knight's square
  if ((bitboard >> 17) & NOT_FILE_H) attacks |= (bitboard >> 17);   // 2U, 1R
  if ((bitboard >> 15) & NOT_FILE_A) attacks |= (bitboard >> 15);   // 2U, 1L
  if ((bitboard >> 10) & NOT_FILE_HG) attacks |= (bitboard >> 10);  // 1U, 1R
  if ((bitboard >> 6) & NOT_FILE_AB) attacks |= (bitboard >> 6);    // 1U, 1L
  if ((bitboard << 17) & NOT_FILE_A) attacks |= (bitboard << 17);   // 2D, 1L
  if ((bitboard << 15) & NOT_FILE_H) attacks |= (bitboard << 15);   // 2D, 1R
  if ((bitboard << 10) & NOT_FILE_AB) attacks |= (bitboard << 10);  // 1D, 1L
  if ((bitboard << 6) & NOT_FILE_HG) attacks |= (bitboard << 6);    // 1D, 1R

  // Return attack bitboard
  return attacks;
}

/**
 * Initializes the pre-computed attack tables for leaper pieces (pawns, knights, and kings) at the start of the program.
 * This function populates global arrays with bitboards representing all possible moves.
 */
void initLeapersAttacks() {
  // Loop over 64 board squares
  for (int square = 0; square < 64; square++) {
    // Init pawn attacks for both colors (two colors, since they move in two different directions)
    pawn_attacks[white][square] = generatePawnAttacks(white, square);
    pawn_attacks[black][square] = generatePawnAttacks(black, square);

    // Init knight attacks
    knight_attacks[square] = generateKnightAttacks(square);

    // Init king attacks
    king_attacks[square] = generateKingAttacks(square);
  }
}