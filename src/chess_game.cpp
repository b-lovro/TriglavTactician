#include "./chess_game.h"

/**
 * Attempts to make a move on the chessboard, updating the game state accordingly.
 * - Checks if the move puts the own king in check, reverting the move if it's illegal.
 * - Updates bitboards, board's flags and move counter.
 *
 * @param move; An integer representing the move to be made.
 * @return Returns true (1) if the move is legal and successfully made, otherwise false (0).
 */
bool ChessGame::MakeMove(int move) {
  // Init move's info
  int from_square, to_square, piece, promoted, capture, enpassant, castling, color, double_p;

  // Copy board's state before move
  board.copyBoard();

  // Get move's info
  from_square = moves.get_move_source(move);
  to_square = moves.get_move_target(move);
  piece = moves.get_move_piece(move);
  promoted = moves.get_move_promoted(move);
  capture = moves.get_move_capture(move);
  enpassant = moves.get_move_enpassant(move);
  castling = moves.get_move_castling(move);
  double_p = moves.get_move_double(move);

  // Update piece bitboards
  pop_bit(board.bitboards[piece], from_square);
  set_bit(board.bitboards[piece], to_square);

  // Update occupancy bitboars
  color = (piece < 6) ? white : black;
  pop_bit(board.occupancy[color], from_square);
  set_bit(board.occupancy[color], to_square);

  if (capture) {
    // Loop trough opponents bitboards
    int range = (color == 0) ? 6 : 0;
    for (int piece = range; piece < (range + 6); piece++) {
      if (get_bit(board.bitboards[piece], to_square)) {
        pop_bit(board.bitboards[piece], to_square);
        break;
      }
    }
    // Update opponents occupancy
    pop_bit(board.occupancy[color ^ 1], to_square);
  }

  // Promotioin move
  if (promoted) {
    pop_bit(board.bitboards[piece], to_square);
    set_bit(board.bitboards[promoted], to_square);
  }

  // On enpassant move, pop bit on row below/above from to_square
  if (enpassant) {
    (color == white) ? pop_bit(board.bitboards[BP], to_square + 8) : pop_bit(board.bitboards[WP], to_square - 8);
    board.enpassant = no_sq;
  }

  if (castling) {
    switch (to_square) {
      // W king side
      case (g1):
        pop_bit(board.bitboards[WR], h1);
        set_bit(board.bitboards[WR], f1);
        break;
      // W queen's side
      case (c1):
        pop_bit(board.bitboards[WR], a1);
        set_bit(board.bitboards[WR], d1);
        break;
      // B king side
      case (g8):
        pop_bit(board.bitboards[BR], h8);
        set_bit(board.bitboards[BR], f8);
        break;
      // B queen's side
      case (c8):
        pop_bit(board.bitboards[BR], a8);
        set_bit(board.bitboards[BR], d8);
        break;
    }
  }

  // ---- UPDATE ----

  // Update castling rights
  board.castling &= CASTLING_RIGHTS[from_square];
  board.castling &= CASTLING_RIGHTS[to_square];

  // Reset occupancy boards
  board.occupancy[white] = 0ULL;
  board.occupancy[black] = 0ULL;

  // Update occupancy boards
  for (int piece = WP; piece <= WK; piece++) {
    board.occupancy[white] |= board.bitboards[piece];
  }
  for (int piece = BP; piece <= BK; piece++) {
    board.occupancy[black] |= board.bitboards[piece];
  }

  board.occupancy[both] = board.occupancy[white] | board.occupancy[black];

  // Check if move is legal
  if (board.isThereCheck(color)) {
    // Undo move, revert to previous board
    board.revertBoard();
    return 0;
  }

  // Update enpassant square if it is double pawn move
  if (double_p) {
    board.enpassant = (color == white) ? from_square - 8 : from_square + 8;
  } else {
    board.enpassant = no_sq;
  }

  // Update color
  board.color = board.color ^ 1;

  // Update move counter
  board.num_moves += 1;

  return 1;
}

// ==============================
//        Perft Testing
// ==============================

void ChessGame::doPerftTest(unsigned int depth) { perftTest(depth, *this); }

// =====================================
//   universal chess interface (UCI)
// =====================================

/**
 * Parses a move string and attempts to match it against generated pseudo-legal moves.
 *
 * @param move_str; String representing the move in standard algebraic notation (e.g., "e2e4", "e7e8q" for promotion).
 * @return Returns the matched move as an integer, or 0 if the move is illegal.
 */
int ChessGame::parseMove(const char *move_str) {
  // Generate moves
  moves.generate_moves(board);

  // Parse to/from squares
  int from_square = (move_str[0] - 'a') + (8 - (move_str[1] - '0')) * 8;
  int to_square = (move_str[2] - 'a') + (8 - (move_str[3] - '0')) * 8;

  // Loop over the moves within a move list
  for (auto &move : moves.moves) {
    // Check move against generated pseudo-legal moves
    if (from_square == moves.get_move_source(move) && to_square == moves.get_move_target(move)) {
      // init promoted piece
      int promoted_piece = moves.get_move_promoted(move);

      // promoted piece is available
      if (promoted_piece) {
        // promoted to queen
        if ((promoted_piece == WQ || promoted_piece == BQ) && move_str[4] == 'q')
          // return legal move
          return move;

        // promoted to rook
        else if ((promoted_piece == WR || promoted_piece == BR) && move_str[4] == 'r')
          // return legal move
          return move;

        // promoted to bishop
        else if ((promoted_piece == WB || promoted_piece == BB) && move_str[4] == 'b')
          // return legal move
          return move;

        // promoted to knight
        else if ((promoted_piece == WN || promoted_piece == BN) && move_str[4] == 'n')
          // return legal move
          return move;

        // continue the loop on possible wrong promotions (e.g. "e7e8f")
        continue;
      }

      // return legal move
      return move;
    }
  }

  // return illegal move
  return 0;
}

/**
 * Parses a FEN string (and a sequence of moves) from a given starting position and updates the board state accordingly.
 * This function has two operations:
 * - Parsing and setting up the board from a given FEN string.
 * - Processing a sequence of moves provided after the FEN string and making those moves on the board.
 * If no FEN is specified or an error is encountered, the board is set to the initial position.
 *
 * @param fen; String containing the FEN representation of the position or "startpos", followed by an optional sequence
 * of moves.
 */
void ChessGame::parsePosition(const char *fen) {
  fen += 9;
  const char *pointer_c = fen;

  if (strncmp(fen, "startpos", 8) == 0) {
    board.parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
  } else {
    pointer_c = strstr(fen, "fen");
    if (pointer_c == NULL) {
      board.parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    } else {
      pointer_c += 4;
      board.parseFEN(pointer_c);
    }
  }

  pointer_c = strstr(fen, "moves");
  int move;

  if (pointer_c != NULL) {
    pointer_c += 6;
    while (*pointer_c) {
      move = parseMove(pointer_c);
      if (!move) {
        break;
      }
      MakeMove(move);

      while (*pointer_c && *pointer_c != ' ') {
        pointer_c++;
      }
      pointer_c++;
    }
  }
}

/**
 * Parses the "go" command from the UCI (Universal Chess Interface) protocol input,
 * acting on specific parameters:search depth or running a perft test or movetime.
 *
 * @param command; The input command string received from the UCI interface.
 */
void ChessGame::parseGo(const char *command) {
  // init parameters
  int depth = 20;
  int max_searched_depth = 20;

  long long remaining_time_ms = Timer::DEFAULT_THINKING_TIME_MS;
  long long increment_time_ms = Timer::DEFAULT_INCREMENT_TIME_MS;

  // Check for "depth" argument in command
  const char *argument = strstr(command, "depth");
  if (argument) {
    depth = atoi(argument + 6);
    if (depth <= 0 || depth > max_searched_depth) {
      std::cout << "Invalid depth specified. Using default depth of " << max_searched_depth << ".\n";
      depth = max_searched_depth;
    }
    // Proceed to search with the specified depth
    this->timer.StartTimer(remaining_time_ms, increment_time_ms);
    searchPosition(*this, depth);
    return;
  }

  // Check for "perft" argument in command
  argument = strstr(command, "perft");
  if (argument) {
    depth = atoi(argument + 6);
    if (depth > 0) {
      doPerftTest(depth);  // Perform a perft test with the specified depth
    } else {
      std::cout << "Please specify a correct depth for the perft test.\n";
    }
    return;  // Exit the function after handling "perft"
  }

  // Check for "movetime" argument in command
  argument = strstr(command, "movetime");
  if (argument) {
    remaining_time_ms = atoi(argument + 9);
    if (remaining_time_ms > 0) {
      increment_time_ms = remaining_time_ms;  // Use remaining time as increment if specified
      this->timer.StartTimer(remaining_time_ms, increment_time_ms);
      searchPosition(*this, depth);  // Start searching with specified time control
      return;
    }
  }

  // If no specific command was found, use default depth and time control to start searching
  if (!argument) {
    std::cout << "Invalid command.\n";
  }
}



/**
 * Initializes the Universal Chess Interface (UCI) protocol.
 * This function processes UCI commands:"isready", "ucinewgame", "position", "go", "help" and "quit".
 * Also processes "print", which just prints current state of the board
 */
void ChessGame::startUCI() {
  // Init input line
  char line[2000];
  // For connection with GUI
  setbuf(stdin, NULL);
  setbuf(stdout, NULL);

  std::cout << MESSAGE << std::endl;

  while (true) {
    memset(&line[0], 0, sizeof(line));
    // For connection with GUI
    fflush(stdout);
    if (!fgets(line, 2000, stdin)) continue;

    if (line[0] == '\n') continue;

    if (!strncmp(line, "isready", 7)) {
      std::cout << "readyok\n";
      continue;
    } else if (!strncmp(line, "print", 5)) {
      printBoard();
    } else if (!strncmp(line, "position", 8)) {
      parsePosition(line);
    } else if (!strncmp(line, "ucinewgame", 10)) {
      parsePosition("position startpos\n");
    } else if (!strncmp(line, "go", 2)) {
      parseGo(line);
    } else if (!strncmp(line, "quit", 4)) {
      break;
    } else if (!strncmp(line, "uci", 3)) {
      std::cout << MESSAGE << std::endl;
    } else if (!strncmp(line, "help", 3)) {
      std::cout << UCI_HELP << std::endl;
    } else {
      std::cout << "Invalid command" << std::endl;
    }
  }
}
