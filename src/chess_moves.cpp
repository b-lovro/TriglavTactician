#include "./chess_moves.h"

// =========================
//  Move Generation Methods
// =========================

/**
 * Generates all possible pawn moves for a given color on the chessboard.
 * (quiet, captures, enpassant, double, promotions)
 *
 * @param board; The current state of the chessboard.
 * @param color; The color of the pawns to generate moves for (white or black).
 * @param occupancy; Representing the occupancy of pieces on the board,
 *                   (0 for white, 1 for black, and 2 for both).
 */
void Moves::generateMovesPawns(ChessBoard board, int color, U64 occupancy[3]) {
  // Init source, target, piece and direction
  int from_square, to_square, piece, direction;

  // Init pieces bitboard and it's attacks
  U64 bitboard, attacks;
  bitboard = (color == white) ? board.bitboards[WP] : board.bitboards[BP];
  piece = (color == white) ? WP : BP;

  // Pawns move direction differes on color.
  direction = (color == white) ? -8 : 8;

  // Get promotions based on color
  const int* promotions = (color == white) ? WHITE_PROMOTIONS : BLACK_PROMOTIONS;

  // Loop over pawns in bitboard
  while (bitboard) {
    // Get from_square (least significant set bit in a bitboard)
    from_square = bitScanForward(bitboard);

    // Init to square
    to_square = from_square + direction;

    // Generate quiet pawn moves
    bool is_move_push_allowed = (color == white) ? !(to_square < a8) : !(to_square > h1);
    if (is_move_push_allowed && !get_bit(occupancy[both], to_square)) {
      // pawn promotion
      bool is_promotion_allowed =
          (color == white) ? (from_square >= a7 && from_square <= h7) : (from_square >= a2 && from_square <= h2);
      if (is_promotion_allowed) {
        for (int i = 0; i < 4; ++i) {
          add_move(encode_move(from_square, to_square, piece, promotions[i], 0, 0, 0, 0));
        }
      } else {
        // one square ahead pawn move
        add_move(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0));

        // two squares ahead pawn move
        bool is_double_allowed =
            (color == white) ? (from_square >= a2 && from_square <= h2) : (from_square >= a7 && from_square <= h7);
        if (is_double_allowed && !get_bit(occupancy[both], to_square + direction))
          add_move(encode_move(from_square, to_square + direction, piece, 0, 0, 1, 0, 0));
      }
    }

    // Get pawn attacks bitboard
    attacks = pawn_attacks[color][from_square] & occupancy[color ^ 1];

    // Loop over target squares in attacks
    while (attacks) {
      // Init to square
      to_square = bitScanForward(attacks);

      // PROMOTIONS
      bool is_promotion_allowed =
          (color == white) ? (from_square >= a7 && from_square <= h7) : (from_square >= a2 && from_square <= h2);
      if (is_promotion_allowed) {
        for (int i = 0; i < 4; ++i) {
          // Add promotion move
          add_move(encode_move(from_square, to_square, piece, promotions[i], 1, 0, 0, 0));
        }
      } else {
        // one square ahead pawn move
        add_move(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0));
      }
      // Pop least significant set bit of the pawn attacks
      pop_bit(attacks, to_square);
    }

    // ENPASSANT
    if (board.enpassant != no_sq) {
      // lookup pawn attacks and bitwise AND with enpassant square
      U64 enpassant_attacks = pawn_attacks[color][from_square] & (1ULL << board.enpassant);

      // Make sure enpassant capture available
      if (enpassant_attacks) {
        // Init enpassant capture target square
        int target_enpassant = bitScanForward(enpassant_attacks);
        // Add enpassant move
        add_move(encode_move(from_square, target_enpassant, piece, 0, 1, 0, 1, 0));
      }
    }

    // Pop least significant set bit from piece bitboard
    pop_bit(bitboard, from_square);
  }
}

/**
 * Generates all possible king moves for a given color on the chessboard.
 * (quiet, captures, castling)
 *
 * @param board; The current state of the chessboard.
 * @param color; The color of the pawns to generate moves for (white or black).
 * @param occupancy; Representing the occupancy of pieces on the board,
 *                   (0 for white, 1 for black, and 2 for both).
 */
void Moves::generateMovesKings(ChessBoard board, int color, U64 occupancy[3]) {
  // Init source,target and piece
  int from_square, to_square, piece;

  // Init pieces bitboard and it's attacks
  U64 bitboard, attacks;
  bitboard = (color == white) ? board.bitboards[WK] : board.bitboards[BK];
  piece = (color == white) ? WK : BK;

  // Loop over kings in bitboard
  while (bitboard) {
    // Get from_square (least significant set bit in a bitboard)
    from_square = bitScanForward(bitboard);

    // Init king' attacks. (attacks from square bitwise AND occupancy of opponent)
    attacks = king_attacks[from_square] & ((color == white) ? ~occupancy[white] : ~occupancy[black]);

    // Loop over target squares in attacks
    while (attacks) {
      // Init to square
      to_square = bitScanForward(attacks);

      if (!get_bit(((color == white) ? occupancy[black] : occupancy[white]), to_square))
        // Add quiet move
        add_move(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0));

      else
        // Add capture move
        add_move(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0));

      // pop ls1b in current attacks set
      pop_bit(attacks, to_square);
    }

    // Pop least significant set bit of the bitboard
    pop_bit(bitboard, from_square);
  }

  // CASTLING
  // Determine squares for king's and queen's side based on the color
  int square_kings_side = (color == white) ? f1 : f8;
  int square_queens_side = (color == white) ? d1 : d8;

  // Check for king's side castling rights based on color
  if (board.castling & ((color == white) ? WK_c : BK_c)) {
    // Ensure squares between the king and rook on the king's side are empty
    if (!get_bit(board.occupancy[both], square_kings_side) && !get_bit(board.occupancy[both], square_kings_side + 1)) {
      // Ensure the king's current square and the square it crosses are not under attack
      if (!board.isSquareAttacked(square_kings_side - 1, color ^ 1) &&
          !board.isSquareAttacked(square_kings_side, color ^ 1)) {
        // Add king's side castling move
        add_move(encode_move(square_kings_side - 1, square_kings_side + 1, piece, 0, 0, 0, 0, 1));
      }
    }
  }

  // Check for queen's side castling rights based on color
  if (board.castling & ((color == white) ? WQ_c : BQ_c)) {
    // Ensure squares between the king and rook on the queen's side are empty
    if (!get_bit(board.occupancy[both], square_queens_side) &&
        !get_bit(board.occupancy[both], square_queens_side - 1) &&
        !get_bit(board.occupancy[both], square_queens_side - 2)) {
      // Ensure the square the king ends up on and the square it crosses are not under attack
      if (!board.isSquareAttacked(square_queens_side + 1, color ^ 1) &&
          !board.isSquareAttacked(square_queens_side, color ^ 1)) {
        // Add queen's side castling move
        add_move(encode_move(square_queens_side + 1, square_queens_side - 1, piece, 0, 0, 0, 0, 1));
      }
    }
  }
}

/**
 * Generates all possible moves for a specified piece on the chessboard.It handles moves for knights,
 * bishops, rooks, and queens. (quiet moves, captures)
 *
 * @param board; The current state of the chessboard.
 * @param occupancy; Representing the occupancy of pieces on the board,
 *                   (0 for white, 1 for black, and 2 for both).
 * @param piece; The specific piece type to generate moves for, identified by its unique code
 *        (e.g., WN for White Knight, BR for Black Rook).
 */
void Moves::generateMovesPiece(ChessBoard board, U64 occupancy[3], unsigned int piece) {
  // Init source, target
  int from_square, to_square;

  // Init pieces bitboard and it's attacks
  U64 bitboard, attacks;

  // Determine the piece+s bitboard based on the piece type
  bitboard = board.bitboards[piece];

  // Loop over pieces in bitboard
  while (bitboard) {
    // Get from_square (least significant set bit in a bitboard)
    from_square = bitScanForward(bitboard);

    // Calculate attacks based on the piece type
    // NOTE: Could be using magic bitboards
    switch (piece) {
      case WN:
      case BN:
        attacks = knight_attacks[from_square];
        break;
      case WB:
      case BB:
        attacks = getBishopMoves(from_square, occupancy[both]);
        break;
      case WR:
      case BR:
        attacks = getRooksMoves(from_square, occupancy[both]);
        break;
      case WQ:
      case BQ:
        attacks = getQueensMoves(from_square, occupancy[both]);
        break;
    }
    // Get piece's attacks bitboard
    attacks &= ((piece < 6) ? ~occupancy[white] : ~occupancy[black]);

    // Loop over target squares in attacks
    while (attacks) {
      // Init to square
      to_square = bitScanForward(attacks);

      if (!get_bit(((piece < 6) ? occupancy[black] : occupancy[white]), to_square))
        // Add quiet move
        add_move(encode_move(from_square, to_square, piece, 0, 0, 0, 0, 0));
      else
        // Add capture move
        add_move(encode_move(from_square, to_square, piece, 0, 1, 0, 0, 0));

      // Pop least significant set bit of the attacks
      pop_bit(attacks, to_square);
    }

    // Pop least significant set bit from piece bitboard
    pop_bit(bitboard, from_square);
  }
}

/**
 * Generates all possible moves for the current player, based on the board's state.
 * Generation of moves for all types of pieces (pawns, knights, bishops, rooks, queens, and kings).
 *
 * @param board The current state of the chessboard, containing bitboards and all flags.
 */
void Moves::generate_moves(ChessBoard board) {
  // Init move count
  moves_count = 0;
  U64 occupancy[3];

  // Copy occupancy boards
  memcpy(occupancy, board.occupancy, sizeof(occupancy));

  if (board.color == white) {
    generateMovesPawns(board, white, occupancy);
    generateMovesKings(board, white, occupancy);

    generateMovesPiece(board, occupancy, WN);
    generateMovesPiece(board, occupancy, WB);
    generateMovesPiece(board, occupancy, WR);
    generateMovesPiece(board, occupancy, WQ);

  } else {
    generateMovesPawns(board, black, occupancy);
    generateMovesKings(board, black, occupancy);

    generateMovesPiece(board, occupancy, BN);
    generateMovesPiece(board, occupancy, BB);
    generateMovesPiece(board, occupancy, BR);
    generateMovesPiece(board, occupancy, BQ);
  }
}

// ================================
//   Debugging and Utility Methods
// ================================

void Moves::add_move(int move) {
  if (moves_count < 256) {
    moves[moves_count] = move;
    moves_count++;
  } else {
    printf("Array is full. Cannot add more moves.\n");
  }
}

// Prints a single move in a readable format, including special move flags.
void Moves::print_move(int move) {
  // Print move
  std::cout << "      " << square_to_position[get_move_source(move)] << square_to_position[get_move_target(move)]
            << ' ';
  if (get_move_promoted(move))
    std::cout << ASCII_PIECES[get_move_promoted(move)];
  else
    std::cout << ' ';
  std::cout << ASCII_PIECES[get_move_piece(move)] << "         " << (get_move_capture(move) ? 1 : 0) << "         "
            << (get_move_double(move) ? 1 : 0) << "         " << (get_move_enpassant(move) ? 1 : 0) << "         "
            << (get_move_castling(move) ? 1 : 0) << "         " << (get_move_promoted(move) ? 1 : 0) << '\n';
}

// Prints all moves stored in the moves array.
void Moves::print_all_moves() {
  // Do nothing on empty move list
  if (moves_count == 0) {
    std::cout << "\n     No move in the move list!\n";
    return;
  }

  std::cout << "\n     move    piece     capture   double    enpass    castling    promotion\n\n";

  // Loop over moves within a move list
  for (auto& move : moves) {
    // Print move
    std::cout << "      " << square_to_position[get_move_source(move)] << square_to_position[get_move_target(move)]
              << ' ';
    if (get_move_promoted(move))
      std::cout << ASCII_PIECES[get_move_promoted(move)];
    else
      std::cout << ' ';
    std::cout << ASCII_PIECES[get_move_piece(move)] << "         " << (get_move_capture(move) ? 1 : 0) << "         "
              << (get_move_double(move) ? 1 : 0) << "         " << (get_move_enpassant(move) ? 1 : 0) << "         "
              << (get_move_castling(move) ? 1 : 0) << "         " << (get_move_promoted(move) ? 1 : 0) << '\n';
  }

  // Print total number of moves
  std::cout << "\n\n     Total number of moves: " << moves_count << "\n\n";
}