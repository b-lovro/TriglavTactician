#ifndef PERFT_H_
#define PERFT_H_

#include <fstream>

#include "./chess_game.h"
#include "./chess_utils.h"

class ChessGame;

long getTimeMs();

void perft(int depth, ChessGame game);
void perftTest(int depth, ChessGame& game);

#endif  // PERFT_H_