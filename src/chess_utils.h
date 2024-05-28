#ifndef CHESS_UTILS_H_
#define CHESS_UTILS_H_

#include <cassert>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>

// Define a type for bitboards.
typedef uint64_t U64;

// --- Chess Piece and Board Definitions ---

// Enumeration for chess pieces for both colors, including an EMPTY value for unoccupied squares.
// clang-format off
enum Piece {
    WP, WN, WB, WR, WQ, WK, // White pieces: Pawns, Knights, Bishops, Rooks, Queens, King
    BP, BN, BB, BR, BQ, BK, // Black pieces
    EMPTY                   // Unoccupied square
};

// Enumeration for the squares of the chessboard, facilitating easy indexing.
enum Square {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
    no_sq // Represents no square or an invalid square
};
// clang-format on

// Mapping from square enum to board position strings.
extern const char* square_to_position[65];

// Enumeration for player colors
enum { white, black, both };

// Enumeration for distinguishing between different pawn moves and attacks.
enum { attack, normal_move };

// --- Bitboard Manipulation Functions ---
// Functions for setting, getting, and clearing bits on a bitboard.
inline void set_bit(U64& bitboard, int square) { bitboard |= (1ULL << square); }
inline bool get_bit(U64& bitboard, int square) { return bitboard & (1ULL << square); }
inline void pop_bit(U64& bitboard, int square) { bitboard &= ~(1ULL << square); }

// --- Utility Functions and Data ---

// Utility functions for converting between different representations.
int charToPieceEnum(char pieceChar);

// ASCII representations of chess pieces for output.
const char ASCII_PIECES[13] = "PNBRQKpnbrqk";
const char ASCII_PIECES_LOWER[13] = "pnbrqkpnbrqk";

// Bit scanning functions to find the least and most significant bits set.
unsigned int bitScanForward(U64 bitboard);
unsigned int bitScanReverse(U64 bitboard);
int countBits(U64 bitboard);

// --- Move Generation Data ---

// File masks for move generation, excluding certain files.
const U64 NOT_FILE_A = 18374403900871474942ULL;
const U64 NOT_FILE_H = 9187201950435737471ULL;
const U64 NOT_FILE_HG = 4557430888798830399ULL;
const U64 NOT_FILE_AB = 18229723555195321596ULL;

// Enumeration for directional rays used in move generation for sliding pieces.
// Rays
enum { UP, DOWN, LEFT, RIGHT, UPLEFT, UPRIGHT, DOWNLEFT, DOWNRIGHT };

// Arrays for directional ray attacks and leaper piece attacks.
extern U64 rays[8][64];
extern U64 pawn_attacks[2][64];
extern U64 knight_attacks[64];
extern U64 king_attacks[64];

// --- Move Generation Functions ---

// Leaper pieces
U64 generatePawnAttacks(unsigned int color, unsigned int square);
U64 generateKingAttacks(unsigned int square);
U64 generateKnightAttacks(unsigned int square);
void initLeapersAttacks();

// Sliding pieces
void initGenerateRays();
U64 getBishopMoves(unsigned int square, U64 blockers);
U64 getRooksMoves(unsigned int square, U64 blockers);
U64 getQueensMoves(unsigned int square, U64 blockers);

// Promotion piece options for white and black.
const int WHITE_PROMOTIONS[4] = {WQ, WR, WB, WN};
const int BLACK_PROMOTIONS[4] = {BQ, BR, BB, BN};

// Look-up tables for reversing bit indices and for castling rights.
// clang-format off
const int INDEX_64_REVERSE[64] = {
  0,  47, 1,  56, 48, 27, 2,  60, 
  57, 49, 41, 37, 28, 16, 3,  61, 
  54, 58, 35, 52, 50, 42, 21, 44, 
  38, 32, 29, 23, 17, 11, 4,  62, 
  46, 55, 26, 59, 40, 36, 15, 53, 
  34, 51, 20, 43, 31, 22, 10, 45, 
  25, 39, 14, 33, 19, 30, 9,  24, 
  13, 18, 8,  12, 7,  6,  5,  63
};

const int INDEX_64_FORWARD[64] = {
  0,  1,  48, 2,  57, 49, 28, 3,  
  61, 58, 50, 42, 38, 29, 17, 4,  
  62, 55, 59, 36, 53, 51, 43, 22, 
  45, 39, 33, 30, 24, 18, 12, 5,  
  63, 47, 56, 27, 60, 41, 37, 16, 
  54, 35, 52, 21, 44, 32, 23, 11, 
  46, 26, 40, 15, 34, 20, 31, 10, 
  25, 14, 19, 9,  13, 8,  7,  6
};

const int CASTLING_RIGHTS[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};
// clang-format on

// Castling rights encoded as bits for easy updating and checking.
enum { WK_c = 1, WQ_c = 2, BK_c = 4, BQ_c = 8 };





//
const std::string WELCOME_MESSAGE = R"(
Welcome to TriglavTactician Chess Engine! 

TriglavTactician: Strategy Peaks Here

Available Commands:
- uci: Start Universal Chess Interface (UCI) mode. (Also has a help command)
- playgame: play a text based game against the engine.
- test [path_to_stockfish_executable]: Run tests against Stockfish engine. Ensure the 'test' subfolder
contains the 'commands.txt' file with test commands.
- help: Display available commands and their descriptions.
- exit: Exit the application.

Enter your command:
)";

const std::string HELP = R"(
Available Commands:
- uci: Start Universal Chess Interface (UCI) mode.
- playgame: play a text based game against the engine.
- test [path_to_stockfish_executable]: This command initiates a series of automated tests against the
Stockfish chess engine. To use this feature, follow these guidelines:

1. Ensure that the 'test' subfolder within your engine's directory contains a file named 'commands.txt'. This
file should list all the chess positions and moves you want to test, formatted according to the UCI
protocol.
2. In 'commands.txt', each new test position should be preceded by the word "NEXT" on its own line. This
marker tells the engine to treat the following lines as a separate test case until it encounters another
"NEXT" marker or reaches the end of the file.
Example:
NEXT
position startpos moves e2e4 e7e5
go depth 10
NEXT
position startpos moves d2d4
go depth 10
3. If you encounter issues running the tests, as a troubleshooting step, clear the 'test' subfolder of all
files except for 'commands.txt'. This can resolve problems related to residual data from previous tests.
Remember to replace [path_to_stockfish_executable] with the actual file path to your Stockfish engine
executable when using the "test" command.

Enter your command:
)";

const std::string UCI_HELP = R"(
Using TriglavTactician with the UCI Protocol:

The UCI protocol facilitates communication between chess engines and GUIs, enabling users to analyze games,
play against the engine, or use the engine to play against other engines. Here are the basic steps and
commands to get started:

1. Start UCI Mode:
- Command: 'uci'
- When you enter 'uci', TriglavTactician will acknowledge UCI mode and provide its identity (name and
author). It will then send 'uciok' to indicate it's ready.

2. New Game:
- Command: 'ucinewgame'
- Use this command at the start of a new game to reset the engine's internal state.

3. Set Position:
- Command: 'position [startpos | fen fenstring] [moves move1 move2 ...]'
- This command sets the current board position. Use 'startpos' for the game's standard start position or
'fen' followed by a FEN string to set a specific position. 
Optionally, you can specify a sequence of moves from the current position using 'moves'.

4. Start Calculating:
- Command: 'go'
- This command tells the engine to start calculating from the current position. 
  - Depth: You can specify the parameter depth to limit the search depth. For example, 'go depth 5' tells the
    engine to calculate using a depth of 5 moves.
  - Movetime: You can specifiy how long the engine should search for the best move in miliseconds.For example, 
    'go movetime 5000' tells the engine to calculate best move in 5s.  
  - Perft: Additionally, you can use 'go perft [depth]' to perform a perft analysis at the specified
    depth. Perft (Performance Test) counts all the possible legal moves up to a certain depth.
    It's a way to verify that the move generation function correctly generates all possible moves. 
    For example, 'go perft 5' will analyze all possible moves from the current position up to 5 moves
    deep.


6. Best Move:
- When the engine has determined the best move based on its calculations, 
it will output 'bestmove [move]', where [move] is the recommended move in UCI move notation (e.g., 'e2e4').

7. Quit:
- Command: 'quit'
- This command exits the engine.

Tips:
- Ensure you're familiar with UCI move notation (e.g., 'e2e4' for pawn to e4, 'e7e8q' for pawn promotion to
queen).
- Use a compatible GUI to easily interact with TriglavTactician without manually typing UCI commands (like
Arena or Banksia).

)";

const std::string GAME_HELP = R"(
_______________________________________________________________________________________________________

Welcome to TriglavTactician Text Game Mode!
Here's how you can command the game:
Starting a New Game:
- Command: 'newgame'
  - Resets the game to the initial chess position.
Making a Move:
- Command: Enter your move in standard chess notation (e.g., 'e2e4' to move a pawn to e4).
Print the Board:
- Command: 'print'
  - Displays the current state of the chessboard.
Requesting Help:
- Command: 'help'
  - Displays available commands and their descriptions.
Quitting the Game:
- Command: 'quit'
  - Exits the game.
Turns:
- Your move is requested after the prompt "Your turn:".

- After your move, TriglavTactician will take its turn, calculating the best move based on the depth or 
  time you've set at the beginning.
Setting Up:
- At the start, you'll choose whether to play as black or white and set the engine's depth or time limit 
  for making moves.
)";



const std::string MESSAGE = R"(
id name TriglavTactician
id author Lovro
uciok
)";



const std::string OUTPUT_FILE_SF = "./test/results_sf.txt";
const std::string OUTPUT_FILE_LB = "./test/results_lb.txt";
const std::string COMMANDS_FILE = "./test/commands.txt";
// Diff format, because it is run in command line
const std::string COMMANDS_FILE_TEMP = "test/commands_temp.txt";

#endif  // CHESS_UTILS_H_