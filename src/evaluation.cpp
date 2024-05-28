#include "./evaluation.h"

int ply = 0;
U64 num_nodes = 0;
int killer_moves[2][64] = {};
int history_moves[12][64] = {};
int pv_length[64] = {};
int pv_table[64][64] = {};

// print move scores DEBUG
void print_move_scores(ChessGame& game) {
  printf("     Move scores:\n\n");

  // loop over moves within a move list
  for (int count = 0; count < game.moves.moves_count; count++) {
    printf("     move: ");
    game.moves.print_move(game.moves.moves[count]);
    printf(" score: %d\n", scoreMove(game, game.moves.moves[count]));
  }
}

void print_move(int move) {
  if (Moves::get_move_promoted(move))
    printf("%s%s%c", square_to_position[Moves::get_move_source(move)], square_to_position[Moves::get_move_target(move)],
           square_to_position[Moves::get_move_promoted(move)]);
  else
    printf("%s%s", square_to_position[Moves::get_move_source(move)], square_to_position[Moves::get_move_target(move)]);
}

/**
 * Evaluates the static value of a given chess board position. This function calculates the
 * material score for both sides and adjusts this score based on the positional value of
 * each piece.
 *
 * @param board The current chess board state.
 * @return The evaluation score of the board. Positive values indicate a better position
 *         for white, while negative values indicate a better position for black.
 */
int Evaluate(ChessBoard board) {
  // Init evaluation score
  int score = 0;

  // Init piece & square
  int piece, square;

  // Loop over piece bitboards
  for (int bb_piece = WP; bb_piece <= BK; bb_piece++) {
    // Init piece bitboard copy
    U64 bitboard = board.bitboards[bb_piece];

    // Loop over pieces within a bitboard
    while (bitboard) {
      piece = bb_piece;                   // Current piece type
      square = bitScanForward(bitboard);  // Find the square of the current piece
      score += material_score[piece];     // Add material score for the piece

      // Add positional score based on piece type
      switch (piece) {
        case WP:
          score += PAWN_SCORE[square];
          break;
        case WN:
          score += KNIGHT_SCORE[square];
          break;
        case WB:
          score += BISHOP_SCORE[square];
          break;
        case WR:
          score += ROOK_SCORE[square];
          break;
        case WK:
          score += KING_SCORE[square];
          break;
        // Use mirrored score for black pieces
        case BP:
          score -= PAWN_SCORE[MIRROR_SCORE[square]];
          break;
        case BN:
          score -= KNIGHT_SCORE[MIRROR_SCORE[square]];
          break;
        case BB:
          score -= BISHOP_SCORE[MIRROR_SCORE[square]];
          break;
        case BR:
          score -= ROOK_SCORE[MIRROR_SCORE[square]];
          break;
        case BK:
          score -= KING_SCORE[MIRROR_SCORE[square]];
          break;
      }

      pop_bit(bitboard, square);  // Move to next piece in bitboard
    }
  }
  // Return score adjusted for current player's perspective
  return (board.color == white) ? score : -score;
}

/**
 * Scores a given move based on its type and strategic value. The function prioritizes captures
 * using the Most Valuable Victim - Least Valuable Attacker (MVV-LVA) heuristic, assigns special
 * scores to killer moves to improve move ordering in the search algorithm, and uses historical
 * move performance for non-captures.
 *
 * @param game; The current state of the chess game.
 * @param move; The move to be scored.
 * @return Integer score representing the move's strategic value.
 */
int scoreMove(ChessGame game, int move) {
  // Check if the move is a capture
  if (Moves::get_move_capture(move)) {
    // for enpassant captures
    int target = WP;  // Default target piece is white pawn
    int to_square = Moves::get_move_target(move);

    // Determine opponent's color
    int oppo_color = game.board.color ^ 1;
    // Determine range of pieces based on the current player's color
    int range = (game.board.color == 0) ? 6 : 0;
    // Loop through opponent's pieces to find the captured piece
    for (int piece = range; piece < (range + 6); piece++) {
      if (get_bit(game.board.bitboards[piece], to_square)) {
        target = piece;
        break;
      }
    }

    // Use MVV-LVA to score captures, adding a high base value to prioritize captures
    return MVV_LVA[Moves::get_move_piece(move)][target] + 10000;
  } else {
    // For non-capture moves, check if the move is a killer move
    /*
        A killer move is a non-capture move that caused a beta-cutoff
        in a sibling node at the same depth of the search tree.
        The rationale is that a move that is effective in one position might
        also be effective in another similar position, even if it doesn't
        involve capturing enemy pieces.
    */

    // Score 1st killer
    if (killer_moves[0][ply] == move) {
      return 9000;
    } else if (killer_moves[1][ply] == move) {
      // Ccore 2nd killer
      return 8000;
    } else {
      // Use historical move performance for non-capture, non-killer moves

      /*
      The history heuristic assigns a score to every possible move based on
      its historical effectiveness in causing alpha-beta cutoffs. The more
      often a move leads to cutoffs, the higher its score, and thats why we
      considered earlyier in future searches
      */
      return history_moves[Moves::get_move_piece(move)][Moves::get_move_target(move)];
    }
  }
  return 0;
}

/**
 * Sorts the moves in a move list based on their scores to improve the efficiency of the search algorithm.
 * This sorting makes better move ordering for alpha-beta pruning by examining potentially stronger moves first.
 *
 * @param game; Rereference to the current game state, which includes the move list to be sorted.
 */
void sortMoves(ChessGame& game) {
  // Array to hold scores for each move in the current move list
  int move_scores[game.moves.moves_count];

  // Assign scores to all moves in the move list
  for (int count = 0; count < game.moves.moves_count; count++) {
    move_scores[count] = scoreMove(game, game.moves.moves[count]);
  }

  // Perform bubble sort on the move list based on move scores
  for (int current_move = 0; current_move < game.moves.moves_count; current_move++) {
    for (int next_move = current_move + 1; next_move < game.moves.moves_count; next_move++) {
      // If the next move has a higher score than the current move, swap them
      if (move_scores[current_move] < move_scores[next_move]) {
        // Swap scores
        int temp_score = move_scores[current_move];
        move_scores[current_move] = move_scores[next_move];
        move_scores[next_move] = temp_score;

        // Swap moves
        int temp_move = game.moves.moves[current_move];
        game.moves.moves[current_move] = game.moves.moves[next_move];
        game.moves.moves[next_move] = temp_move;
      }
    }
  }
}

/**
 * Performs a Quiescence Search on the current game position. It is a technique used to
 * avoid the horizon effect by only evaluating 'quiet' positions, or positions where
 * there are no pending tactical threats. This function extends the search in positions with
 * potential captures, aiming to reach a position stable enough to evaluate safely.
 *
 * The horizon effect can lead to situations where a chess engine makes a move that
 * looks good in the short term but leads to disadvantages later on.
 *
 * @param game; The current state of the chess game.
 * @param alpha; The lower bound of the search window.
 * @param beta; The upper bound of the search window.
 * @return Evaluation score of the position.
 */
int quSearch(ChessGame game, int alpha, int beta) {
  num_nodes++;
  // Evaluate the value of the current board position.
  int eval = Evaluate(game.board);

  // Fail-hard beta cutoff: if the evaluation is greater than or equal to beta,
  // the position is too good and the opponent is unlikely to allow it.
  if (eval >= beta) {
    return beta;
  }

  // If the evaluation is greater than alpha, we have found a better move.
  if (eval > alpha) {
    // Update alpha to the new evaluation score.
    alpha = eval;
  }

  game.moves.generate_moves(game.board);
  // Sort moves to prioritize captures, which are more relevant in quiescence search.
  sortMoves(game);

  for (int i = 0; i < game.moves.moves_count; i++) {
    if (game.timer.IsTimeOut()) {
      break;
    }
    // Focus on capture moves only
    if (game.moves.get_move_capture(game.moves.moves[i])) {
      game.board.copyBoard();
      ply++;

      // Attempt to make the move, skip if it's illegal.
      if (!game.MakeMove(game.moves.moves[i])) {
        ply--;  // Revert ply if the move is not made.
        // boardRevert() is already done in makeMove()
        continue;
      }

      // Recursively call quiescence search with negated and flipped alpha-beta bounds.
      int score = -quSearch(game, -beta, -alpha);

      game.board.revertBoard();
      ply--;

      // Fail-hard beta cutoff check after making the capture move.
      if (score >= beta) {
        return beta;
      }

      // If the score from the capture move is better than alpha, update alpha.
      if (score > alpha) {
        alpha = score;
      }
    }
  }
  // Return the best score found.
  return alpha;
}

/**
 * Performs a Negamax search to a specified depth, evaluating the chess position
 * from the perspective of the current player. Negamax is a variant of the minimax
 * algorithm that relies on the zero-sum property of chess to simplify the implementation.
 * The function also incorporates alpha-beta pruning to improve search efficiency and
 * quiescence search to avoid the horizon effect.
 *
 * @param game; Current game state including the board, move list, and other relevant information.
 * @param alpha; The lower bound of the search window.
 * @param beta; The upper bound of the search window.
 * @param depth; The depth to which the search should go.
 * @return Score of the board from the current player's perspective.
 */
int NegaMax(ChessGame game, int alpha, int beta, int depth) {
  // Initialize the Principal Variation length for the current ply.
  pv_length[ply] = ply;

  // Base case: if search has reached desired depth, evaluate the position
  // using quiescence search to avoid overlooking tactics at the horizon.
  if (depth == 0) {
    return quSearch(game, alpha, beta);
  }

  int in_check = game.board.isThereCheck(game.board.color);

  // increase search depth if king in check to ensure
  // all checks are addressed in the search
  if (in_check) {
    depth++;
  }

  // Tracks the number of legal moves found.
  int legal_moves = 0;

  game.moves.generate_moves(game.board);
  sortMoves(game);  // Sort moves to improve search efficiency.

  num_nodes++;

  // Iterate through all generated moves.
  for (int i = 0; i < game.moves.moves_count; i++) {
    game.board.copyBoard();
    ply++;
    if (game.timer.IsTimeOut()) {
      break;
    }
    // Attempt to make the move, skip if it's illegal.
    if (!game.MakeMove(game.moves.moves[i])) {
      ply--;  // Revert ply if the move is not made.
      // boardRevert() is already done in makeMove()
      continue;
    }

    // increment legal moves
    legal_moves++;

    // Recurse with the negated alpha and beta values, decreasing depth.
    int score = -NegaMax(game, -beta, -alpha, depth - 1);

    game.board.revertBoard();
    ply--;

    // Fail-hard beta cutoff: stop searching if we find a move that's too good.
    if (score >= beta) {
      // Update killer moves if the move is a quiet move (non-capture).
      if (!Moves::get_move_capture(game.moves.moves[i])) {
        killer_moves[1][ply] = killer_moves[0][ply];
        killer_moves[0][ply] = game.moves.moves[i];
      }

      return beta;  // Move is too good; opponent won't allow it.
    }

    // Found a better move, update alpha.
    if (score > alpha) {
      // Update history table if it's a quiet move.
      if (!Moves::get_move_capture(game.moves.moves[i])) {
        history_moves[Moves::get_move_piece(game.moves.moves[i])][Moves::get_move_target(game.moves.moves[i])] += depth;
      }
      alpha = score;

      // Update Principal Variation (PV) table.
      pv_table[ply][ply] = game.moves.moves[i];
      for (int n_ply = ply + 1; n_ply < pv_length[ply + 1]; n_ply++) {
        // Copy move from deeper ply into current ply's line
        pv_table[ply][n_ply] = pv_table[ply + 1][n_ply];
      }

      pv_length[ply] = pv_length[ply + 1];
    }
  }

  // If no legal moves were found, check for checkmate or stalemate.
  if (legal_moves == 0) {
    if (in_check) {
      // Checkmate condition: negative score indicating loss, adjusted by ply
      // to favor delaying the loss as long as possible.
      return -49000 + ply;
    } else {
      // Stalemate condition: return 0 score.
      return 0;
    }
  }

  // Return the best score found for this node.
  return alpha;
}

/**
 * Initiates a search on the given chess position up to a specified depth, using the Negamax algorithm.
 * It evaluates the position and decides on the best move, printing the search results.(score,depth,num_nodes,PV
 * sequence)
 *
 * @param game; Current state of the chess game.
 * @param depth; Depth to which the search algorithm should explore the tree.
 */
void searchPosition(ChessGame& game, unsigned int depth) {
  num_nodes = 0;               // Reset the global nodes counter
  ply = 0;                     // Reset the global depth counter
  ChessGame game_temp = game;  // Copy of the game state, so we don't change it
  int score;
  int alpha = -50000;
  int beta = 50000;

  // Perform the Negamax search
  // Extreme alpha, beta values ensure  the search explores all possible outcomes within the specified depth.

  // Added iterative deepening
  for (int curr_depth = 1; curr_depth <= depth; curr_depth++) {
    if (game.timer.IsTimeOut()) {
      break;
    }

    score = NegaMax(game_temp, alpha, beta, curr_depth);

    // we fell outside the window, so try again with a full-width window (and the same depth)
    if ((score <= alpha) || (score >= beta)) {
      alpha = -50000;
      beta = 50000;
      continue;
    }
    // set up the window for the next iteration
    alpha = score - 50;
    beta = score + 50;

    // Print search information: score (in centipawns), search depth, and total nodes visited.
    std::cout << "info score cp " << score << " depth " << curr_depth << " nodes " << num_nodes << " pv ";
    // Print the Principal Variation: the sequence of best moves found during the search.
    for (int move = 0; move < pv_length[0]; move++) {
      print_move(pv_table[0][move]);
      std::cout << " ";
    }
    std::cout << "\n";
  }

  std::cout << " ";
  std::cout << "bestmove ";
  print_move(pv_table[0][0]);
  std::cout << "\n ";
  game.best_move = pv_table[0][0];
}
