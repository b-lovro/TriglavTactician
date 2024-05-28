#include "./chess_board.h"

// =================================
//         State Management
// =================================

void ChessBoard::copyBoard() {
  // Copy current bitboard
  memcpy(bitboards_copy, bitboards, sizeof(bitboards_copy));
  memcpy(occupancy_copy, occupancy, sizeof(occupancy_copy));

  // Copy current flags
  color_copy = color;
  enpassant_copy = enpassant;
  castling_copy = castling;
}

void ChessBoard::revertBoard() {
  // Revert to previous biboards
  memcpy(bitboards, bitboards_copy, sizeof(bitboards));
  memcpy(occupancy, occupancy_copy, sizeof(occupancy));

  // Revert to previous flags
  color = color_copy;
  enpassant = enpassant_copy;
  castling = castling_copy;
}

void ChessBoard::resetBoard() {
  // Clear all bitboards
  for (int piece = WP; piece <= BK; piece++) {
    bitboards[piece] = 0ULL;
    bitboards_copy[piece] = 0ULL;
  }
  for (int color = white; color < both; color++) {
    occupancy[color] = 0ULL;
    occupancy_copy[color] = 0ULL;
  }
  // Clear all flags
  color = 0;
  color_copy = both;
  enpassant = no_sq;
  enpassant_copy = no_sq;
  castling = 0;
  castling_copy = 0;
  num_moves = 0;
}

// =================================
//      Board and Move Analysis
// =================================

bool ChessBoard::isSquareAttacked(int square, int color) {
  return (
             // Pawn attacks
             ((color == white) ? (pawn_attacks[black][square] & bitboards[WP])
                               : (pawn_attacks[white][square] & bitboards[BP])) ||
             // Knight attacks
             (knight_attacks[square] & ((color == white) ? bitboards[WN] : bitboards[BN])) ||
             // Bishop attacks
             (getBishopMoves(square, occupancy[both]) & ((color == white) ? bitboards[WB] : bitboards[BB])) ||
             // Rook attacks
             (getRooksMoves(square, occupancy[both]) & ((color == white) ? bitboards[WR] : bitboards[BR])) ||
             // Queen attacks
             (getQueensMoves(square, occupancy[both]) & ((color == white) ? bitboards[WQ] : bitboards[BQ])) ||
             // King attacks
             (king_attacks[square] & ((color == white) ? bitboards[WK] : bitboards[BK])))
             ? 1
             : 0;
}

bool ChessBoard::isThereCheck(int color) {
  return isSquareAttacked((color == white) ? bitScanForward(bitboards[WK]) : bitScanForward(bitboards[BK]), color ^ 1);
}

// =================================
//          Board Setup
// =================================

void ChessBoard::parseFEN(const char *fen) {
  resetBoard();

  // loop over board ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over board files
    for (int file = 0; file < 8; file++) {
      // init current square
      int square = rank * 8 + file;

      // match ascii pieces within FEN string
      if ((*fen >= 'a' && *fen <= 'z') || (*fen >= 'A' && *fen <= 'Z')) {
        // init piece type
        int piece = charToPieceEnum(*fen);

        // set piece on corresponding bitboard
        set_bit(bitboards[piece], square);

        // increment pointer to FEN string
        fen++;
      }

      // match empty square numbers within FEN string
      if (*fen >= '0' && *fen <= '9') {
        // init offset (convert char 0 to int 0)
        int offset = *fen - '0';

        // define piece variable
        int piece = -1;

        // loop over all piece bitboards
        for (int bb_piece = WP; bb_piece <= BK; bb_piece++) {
          // if there is a piece on current square
          if (get_bit(bitboards[bb_piece], square))
            // get piece code
            piece = bb_piece;
        }

        // on empty current square
        if (piece == -1)
          // decrement file
          file--;

        // adjust file counter
        file += offset;

        // increment pointer to FEN string
        fen++;
      }

      // match rank separator
      if (*fen == '/')
        // increment pointer to FEN string
        fen++;
    }
  }

  // got to parsing side to move (increment pointer to FEN string)
  fen++;

  // parse side to move
  (*fen == 'w') ? (this->color = white) : (this->color = black);

  // go to parsing castling rights
  fen += 2;

  // parse castling rights
  while (*fen != ' ') {
    switch (*fen) {
      case 'K':
        this->castling |= WK_c;
        break;
      case 'Q':
        this->castling |= WQ_c;
        break;
      case 'k':
        this->castling |= BK_c;
        break;
      case 'q':
        this->castling |= BQ_c;
        break;
      case '-':
        break;
    }

    // increment pointer to FEN string
    fen++;
  }

  // got to parsing enpassant square (increment pointer to FEN string)
  fen++;

  // parse enpassant square
  if (*fen != '-') {
    // parse enpassant file & rank
    int file = fen[0] - 'a';
    int rank = 8 - (fen[1] - '0');

    // init enpassant square
    this->enpassant = rank * 8 + file;
  }

  // no enpassant square
  else
    this->enpassant = no_sq;

  // loop over white pieces bitboards
  for (int piece = WP; piece <= WK; piece++)
    // populate white occupancy bitboard
    occupancy[white] |= bitboards[piece];

  // loop over black pieces bitboards
  for (int piece = BP; piece <= BK; piece++)
    // populate white occupancy bitboard
    occupancy[black] |= bitboards[piece];

  // init all occupancies
  occupancy[both] = occupancy[white] | occupancy[black];
}

// =================================
//       Board Visualization
// =================================

void ChessBoard::printBoard() {
  printf("\n");

  // loop over board ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop ober board files
    for (int file = 0; file < 8; file++) {
      // init square
      int square = rank * 8 + file;

      // print ranks
      if (!file) printf("  %d ", 8 - rank);

      // define piece variable
      int piece = -1;

      // loop over all piece bitboards
      for (int bb_piece = WP; bb_piece <= BK; bb_piece++) {
        // if there is a piece on current square
        if (get_bit(bitboards[bb_piece], square))
          // get piece code
          piece = bb_piece;
      }

      printf(" %c", (piece == -1) ? '.' : ASCII_PIECES[piece]);
    }

    // print new line every rank
    printf("\n");
  }

  // print board files
  printf("\n     a b c d e f g h \n\n");

  //DEBUG
  // // print color to move
  // printf("     Color:     %s\n", color ? "black" : "white");

  // // print enpassant square
  // printf("     Enpassant:   %s\n", (enpassant != no_sq) ? square_to_position[enpassant] : "--");

  // // print castling rights
  // printf("     Castling:  %c%c%c%c\n\n", (castling & WK_c) ? 'K' : '-', (castling & WQ_c) ? 'Q' : '-',
  //        (castling & BK_c) ? 'k' : '-', (castling & BQ_c) ? 'q' : '-');
}

// DEBUG
void ChessBoard::printBitBoard(U64 bitboard) {
  std::cout << "\n";

  // loop over board ranks
  for (int rank = 0; rank < 8; rank++) {
    // loop over board files
    for (int file = 0; file < 8; file++) {
      // convert file & rank into square index
      int square = rank * 8 + file;

      // print ranks
      if (!file) std::cout << "  " << 8 - rank << " ";

      // print bit state (either 1 or 0)
      std::cout << " " << (get_bit(bitboard, square) ? 1 : 0);
    }

    // print new line every rank
    std::cout << "\n";
  }

  // print board files
  std::cout << "\n     a b c d e f g h  <-- files\n\n";

  // print bitboard as unsigned decimal number
  std::cout << "     Bitboard: " << bitboard << "\n\n";
}