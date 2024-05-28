#ifndef CHESS_BOARD_H_
#define CHESS_BOARD_H_

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>

#include "./chess_utils.h"

class ChessBoard {
 public:
  // Bitboards for each piece type and color and white/black/both occupancy
  U64 bitboards[12];
  U64 occupancy[3];

  // Copies of the bitboards and occupancy for undo functionality.
  U64 bitboards_copy[12];
  U64 occupancy_copy[3];

  // Flags
  unsigned int color, color_copy;
  unsigned int enpassant, enpassant_copy;
  unsigned int castling, castling_copy;
  // Move counter
  unsigned int num_moves;

  // Default constructor
  ChessBoard() {
    // Initialize an empty board
    resetBoard();
  }

  // Constructor with a board fen representation
  explicit ChessBoard(const char *fen) { parseFEN(fen); }

  // --- Board Setup ---
  void parseFEN(const char *fen);

  // --- State Management ---
  void copyBoard();
  void revertBoard();
  void resetBoard();

  // --- Board and Move Analysis ---
  bool isSquareAttacked(int square, int side);
  bool isThereCheck(int color);

  // --- Board Visualization ---
  void printBitBoard(U64 bitboard); //DEBUG
  void printBoard();


  
};

#endif  // CHESS_BOARD_H_