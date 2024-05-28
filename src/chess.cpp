#include <sstream>

#include "./chess_game.h"
#include "./chess_game_ter.h"
int main() {
  std::cout << WELCOME_MESSAGE << std::endl;

  std::string command;

  while (true) {
    if (!std::getline(std::cin, command)) break;

    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    if (cmd == "uci") {
      ChessGame game;
      game.startUCI();
    } else if (cmd == "test") {
      ChessGame game;
      std::string path_to_file;
      iss >> path_to_file;
      game.testAgainstSF(path_to_file);
    } else if (cmd == "playgame") {
      ChessGameTER game;
      game.startGameTER();

    } else if (cmd == "help") {
      std::cout << HELP << std::endl;
    } else if (cmd == "exit") {
      break;
    } else {
      std::cout << "Unknown command." << std::endl;
    }
  }

  return 0;
}
