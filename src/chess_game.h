#ifndef CHESS_GAME_H_
#define CHESS_GAME_H_

#include <string>

#include "./chess_board.h"
#include "./chess_moves.h"
#include "./chess_utils.h"
#include "./evaluation.h"
#include "./perft.h"
#include "./chess_timer.h"

class ChessGame {
 public:
  ChessBoard board;
  Moves moves;
  bool file_output;  // for running tests
  int best_move;

  Timer timer;
  // Constructor
  ChessGame() : ChessGame("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1") {}

  // Constructor to initialize the game with a FEN string
  ChessGame(const char *fen) {
    initLeapersAttacks();
    initGenerateRays();

    board.parseFEN(fen);

    this->board = board;
    this->file_output = false;
  }

  // --- Print Board ---
  void printBoard() { board.printBoard(); }

  // --- Move Utilities ---
  bool MakeMove(int move);
  void undoLastMove() {
    if (board.num_moves != 0) {
      board.revertBoard();
    }
  }

  // --- UCI ---
  void startUCI();

  // --- testing ---
  void testAgainstSF(std::string &path_to_sf);

  // --- Perft testing ---
  void doPerftTest(unsigned int depth);

  // --- UCI ---
  void parsePosition(const char *fen);
  int parseMove(const char *ptrChar);
  void parseGo(const char *command);
};

#endif  // CHESS_GAME_H_