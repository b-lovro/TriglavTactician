#include "./chess_game.h"

class ChessGameTER : public ChessGame {
 public:
  int color_player;
  int depth_player;
  int time_player;
  std::string best_move_str;
  // Constructor
  ChessGameTER() : ChessGame(), color_player(0), depth_player(0) {}

  // Constructor that takes a FEN string and initializes ChessGame with it
  ChessGameTER(const char *fen) : ChessGame(fen), color_player(0), depth_player(0) {}

  // --- Utility ---
  void handleUserInput();
  int countLegalMoves();

  // --- Game Loop ---
  void startGameTER();
};