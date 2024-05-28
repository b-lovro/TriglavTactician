#ifndef MOVE_ENCODE_H_
#define MOVE_ENCODE_H_

#include <cstring>
#include <stack>
#include <string>

#include "./chess_board.h"
#include "./chess_utils.h"

/// A move needs 16 bits to be stored
///
/// bit  0- 5: destination square (from 0 to 63)
/// bit  6-11: origin square (from 0 to 63)
/// bit 12-13: promotion piece type - 2 (from KNIGHT-2 to QUEEN-2)
/// bit 14-15: special move flag: promotion (1), en passant (2), castling (3)
/// NOTE: EN-PASSANT bit is set only when a pawn can be captured
///

/*
          binary move bits                               hexidecimal constants

    0000 0000 0000 0000 0011 1111    source square       0x3f
    0000 0000 0000 1111 1100 0000    target square       0xfc0
    0000 0000 1111 0000 0000 0000    piece               0xf000
    0000 1111 0000 0000 0000 0000    promoted piece      0xf0000
    0001 0000 0000 0000 0000 0000    capture flag        0x100000
    0010 0000 0000 0000 0000 0000    double push flag    0x200000
    0100 0000 0000 0000 0000 0000    enpassant flag      0x400000
    1000 0000 0000 0000 0000 0000    castling flag       0x800000
*/

class Moves {
 public:
  int moves[265];
  unsigned int moves_count;

  // --- Move Decoding Utilities ---
  static inline int get_move_source(int move) { return (move & 0x3f); }
  static inline int get_move_target(int move) { return (move & 0xfc0) >> 6; }
  static inline int get_move_piece(int move) { return (move & 0xf000) >> 12; }
  static inline int get_move_promoted(int move) { return (move & 0xf0000) >> 16; }
  static inline int get_move_capture(int move) { return move & 0x100000; }
  static inline int get_move_double(int move) { return move & 0x200000; }
  static inline int get_move_enpassant(int move) { return move & 0x400000; }
  static inline int get_move_castling(int move) { return move & 0x800000; }

  // --- Move Encoding Utilities ---
  inline int encode_move(unsigned int from, unsigned int to, unsigned int piece, unsigned int promoted,
                         unsigned int capture, unsigned int double_p, unsigned int enpassant, unsigned int castling) {
    return (from) | (to << 6) | (piece << 12) | (promoted << 16) | (capture << 20) | (double_p << 21) |
           (enpassant << 22) | (castling << 23);
  }

  // --- Move Generation Methods ---
  void generateMovesPawns(ChessBoard board, int color, U64 occupancy[3]);
  void generateMovesKings(ChessBoard board, int color, U64 occupancy[3]);
  void generateMovesPiece(ChessBoard board, U64 occupancy[3], unsigned int piece);
  void generate_moves(ChessBoard board);

  // --- Utility Methods ---
  void add_move(int move);
  
  // --- Debugging Methods ---
  void print_move(int move);
  void print_all_moves();
};

#endif  // MOVE_ENCODE_H_