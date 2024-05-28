#include "./chess_game_ter.h"

void clearInputStream() {
  std::cin.clear();                                                    
  std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');  
}

/**
 * Handles user input for choosing color, mode and setting search time or depth.
 */
void ChessGameTER::handleUserInput() {
  // Init depth
  depth_player = -1;
  // Init time
  time_player = -1;
  // Init color
  std::string color;
  // Init mode
  std::string mode;

  // Color
  std::cout << "Which color do you want to play as? (black/white): ";
  std::getline(std::cin, color);

  while (color != "black" && color != "white") {
    std::cout << "Invalid color. Please choose 'black' or 'white': ";
    clearInputStream();
    std::getline(std::cin, color);
  }

  color_player = (color == "white") ? white : black;

  // Mode
  std::cout << "How do you want to limit the engine? (time/nodes): ";
  std::getline(std::cin, mode);

  while (mode != "time" && mode != "nodes") {
    std::cout << "Invalid mode. Please choose 'time' or 'nodes': ";
    clearInputStream();
    std::getline(std::cin, mode);
  }

  if (mode == "time") {
    // Time
    std::cout << "Choose time engine can take to calculate best move (in ms): ";
    while (!(std::cin >> time_player)) {
      std::cout << "Invalid time. Please enter a positive number of ms.";
      clearInputStream();
    }
  } else {
    // Depth
    std::cout << "Choose depth to limit the engine (1-9): ";
    while (!(std::cin >> depth_player) || depth_player < 1 || depth_player > 9) {
      std::cout << "Invalid depth. Please enter a number between 1 and 9: ";
      clearInputStream();
    }
  }
}

/**
 * This function generates all possible moves and checks each for legality,
 * incrementing a counter for each legal move found. Usefull for checkmate testing
 * @return int of number of legal moves available.
 */
int ChessGameTER::countLegalMoves() {
  int legal_moves = 0;
  // Generate all possible moves from the current board
  moves.generate_moves(board);
  // Loop through all moves.
  for (int i = 0; i < moves.moves_count; i++) {
    board.copyBoard();

    // If the move is legal, increment the legal_moves counter.
    if (MakeMove(moves.moves[i])) {
      legal_moves++;
      board.revertBoard();
    }
  }

  return legal_moves;
}

/**
 * Starts the text-based chess game, handling game flow and user interaction.
 */
void ChessGameTER::startGameTER() {
  std::cout << GAME_HELP << std::endl;
  // Get user input for player color and engine depth.
  handleUserInput();
  printBoard();

  // Init input line
  char line[2000];
  // Init parsed moves from user input.
  int move;

  // Init parse go command based on mode the player choose
  std::string go_command =
      (depth_player == -1) ? "go movetime " + std::to_string(time_player) : "go depth " + std::to_string(depth_player);

  while (true) {
    memset(&line[0], 0, sizeof(line));

    // Check for game-ending conditions.
    if (board.isThereCheck(color_player) && countLegalMoves() == 0) {
      std::cout << "Checkmate. You lost." << std::endl;
      return;
    }

    // Check for player's turn based on color
    if ((board.color == white && color_player == 0) || (board.color == black && color_player == 1)) {
      std::cout << "Your turn: ";
      if (!fgets(line, sizeof(line), stdin)) continue;

      if (line[0] == '\n')  // Skip empty inputs
        continue;

      // Handle commands
      if (!strncmp(line, "print", 4)) {
        printBoard();
      } else if (!strncmp(line, "quit", 4)) {
        return;
      } else if (!strncmp(line, "help", 4)) {
        std::cout << GAME_HELP << std::endl;
      } else if (!strncmp(line, "newgame", 7)) {
        board.parseFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        startGameTER();
      } else {
        move = parseMove(line);
        if (move != 0 && MakeMove(move)) {
          printBoard();
        } else {
          std::cout << "Invalid move or command." << std::endl;
          if (board.isThereCheck(color_player)) {
            std::cout << "You are in check." << std::endl;
          }
        }
      }
    } else {
      // Engine's turn
      std::cout << "Engine is thinking..." << std::endl;

      // Searching next best move based on mode specified by user
      parseGo(go_command.c_str());

      // If engine return 0, means no legal available
      if (best_move == 0) {
        std::cout << "Stalemate or checkmate. Game over." << std::endl;
        return;
      }

      // If the move is legal, execute it
      if (MakeMove(best_move)) {
        std::cout << "Engine move: ";
        print_move(best_move);
        printBoard();
      } else {
        std::cout << "Engine failed to make a valid move. Check game state." << std::endl;
      }
    }
  }
}