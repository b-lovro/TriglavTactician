#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "./chess_game.h"

// ======================
//        TESTING
// ======================

enum { triglav, stockfish };

const char *engines_str[2] = {"TriglavTactician ", "StockFish "};

/**
 * Represents a block of commands, read from a file, that specify a position and a set of commands to analyze that
 * position.
 */
struct CommandsBlock {
  std::string position;  // The 'position' command specifying the starting position.
  std::vector<std::string>
      go_depth;  // A list of 'go' commands with depth or perft analysis to perform on the position.
};

/**
 * Represents the results of a performance test (Perft) for a specific position.
 */
struct PerftResult {
  int engine;       // Identifier for the chess engine used.
  std::string fen;  // The FEN string representing the chess position.
  int depth;        // The depth to which the Perft test was conducted.
  std::vector<std::pair<std::string, int>>
      move_node;  // A list of moves and the number of nodes (positions) resulting from each move.
  int num_nodes;  // The total number of nodes (positions) evaluated during the Perft test.

  // Custom operators

  bool operator==(const PerftResult &other) const { return fen == other.fen && depth == other.depth; }
  bool operator<(const PerftResult &other) const { return num_nodes < other.num_nodes; }
  bool operator>(const PerftResult &other) const { return num_nodes > other.num_nodes; }
};

std::string ensureCorrectPathFormat(std::string &path) {
  std::string correctedPath;
  for (size_t i = 0; i < path.length(); ++i) {
    if (path[i] == '\\') {
      // Append an extra backslash to escape it correctly
      correctedPath += "\\\\";
      // If the next character is also a backslash, skip it to avoid doubling again
      if (i + 1 < path.length() && path[i + 1] == '\\') {
        i++;
      }
    } else {
      correctedPath += path[i];
    }
  }
  return correctedPath;
}

void printPerftResults(const std::vector<PerftResult> &results) {
  std::cout << "___________________________________________\n";
  std::cout << "PERFT RESULTS FOR ENGINE: " << engines_str[results[0].engine] << "\n\n";

  for (const auto &result : results) {
    std::cout << "---------------------\n";
    std::cout << "FEN: " << result.fen << "\n";
    std::cout << "Depth: " << result.depth << "\n";

    std::cout << "Moves and Nodes:"
              << "\n";
    for (const auto &moveAndNodes : result.move_node) {
      std::cout << moveAndNodes.first << ": " << moveAndNodes.second << "\n";
    }
    std::cout << "Number of Nodes: " << result.num_nodes << "\n\n";
    std::cout << "---------------------\n";
  }
  std::cout << "___________________________________________\n";
}

std::pair<std::string, int> parseMoveAndNodes(const std::string &line) {
  std::istringstream iss(line);
  std::string move;
  int nodes;

  getline(iss, move, ':');
  iss >> nodes;

  return {move, nodes};
}

/**
 * Parses a file containing blocks of commands for analyzing chess positions. Each block starts with the keyword 'NEXT'
 * and includes a position setup followed by one or more analysis commands.
 * This function reads the file and organizes the commands into structured blocks for processing.
 *
 * @param filePath; Path to the file containing the command blocks.
 * @return Vector of CommandsBlock structures.
 */
std::vector<CommandsBlock> parseCommandsBlocks(const std::string &filePath) {
  std::vector<CommandsBlock> blocks;
  std::ifstream file(filePath);
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filePath << std::endl;
    return blocks;
  }

  CommandsBlock currentBlock;
  bool blockStarted = false;

  while (std::getline(file, line)) {
    if (line == "NEXT") {
      if (blockStarted) {
        blocks.push_back(currentBlock);  // Save the previous block before starting a new one
        currentBlock = CommandsBlock();  // Reset the current block
      }
      blockStarted = true;  // Indicate that we are inside a block
    } else if (line.find("position") != std::string::npos) {
      currentBlock.position = line;
    } else if (line.find("go perft") != std::string::npos) {
      currentBlock.go_depth.push_back(line);
    } else if (line == "quit") {
      break;  // Stop parsing if "quit" is encountered
    }
  }

  if (blockStarted) {  // If there was a block being processed, add it to the list
    blocks.push_back(currentBlock);
  }

  return blocks;
}

/**
 * Parses a series of chess position analysis commands and executes them using the Stockfish engine.
 * It captures the output, specifically the performance test (Perft) results, and organizes them into a structured
 * format.
 *
 * @param blocks; Vector of CommandsBlock, each representing a set of commands for analyzing specific positions.
 * @return Vector of PerftResult, containing results of the performance tests for each command block.
 */
std::vector<PerftResult> parsePerftResultsSF(std::vector<CommandsBlock> blocks, std::string path_to_stockfish) {
  // Init an empty vector to hold the Perft results.
  std::vector<PerftResult> results_array;
  // Declare a variable to store the current result.
  PerftResult currentResult;

  // Iterate over each command block provided as input.
  for (const auto &block : blocks) {
    for (const auto &cmd : block.go_depth) {
      // Open a temporary file for writing commands, truncating it first if it exists.
      std::ofstream fileCommandTemp(COMMANDS_FILE_TEMP, std::ofstream::out | std::ofstream::trunc);
      if (!fileCommandTemp.is_open()) {
        std::cerr << "Error: Failed to open temporary command file for writing: " << COMMANDS_FILE_TEMP << std::endl;
        // Continuing to the next command block if unable to open the file
        continue;
      }

      // Write the position and command to the temporary file, then close it.
      fileCommandTemp << block.position + "\n";
      fileCommandTemp << cmd + "\n";
      fileCommandTemp << "quit\n";  // Ensuring Stockfish exits after processing the command
      fileCommandTemp.close();
      // std::cout << "Commands written to " << COMMANDS_FILE_TEMP << " and file closed." << std::endl;

      // Construct the command string to execute Stockfish with input and output redirection.
      std::string command;
      // clang-format off
      #ifdef _WIN32
        // For Windows systems
        command = path_to_stockfish + " < " + COMMANDS_FILE_TEMP + " > " + OUTPUT_FILE_SF;
      #else
        // For Unix-based systems (Linux, macOS)
        command = "./"+ path_to_stockfish +" < " + COMMANDS_FILE_TEMP + " > " + OUTPUT_FILE_SF;
      #endif
      // clang-format on

      // Execute the command in the system shell.
      int result = system(command.c_str());

      // Immediately attempt to delete the temporary command file
      if (std::remove(COMMANDS_FILE_TEMP.c_str()) != 0) {
        std::cerr << "Warning: Failed to delete temporary command file: " << COMMANDS_FILE_TEMP << std::endl;
      }
      // else {
      //   std::cout << "Temporary command file " << COMMANDS_FILE_TEMP << " deleted successfully." << std::endl;
      // }

      if (result != 0) {
        std::cerr << "Error: Command execution failed with return code " << result << std::endl;
        // Skip parsing and move to the next command if execution failed
        continue;
      }

      // Extract the position/FEN and assign
      auto pos = block.position.find("fen");
      currentResult.fen = (pos != std::string::npos) ? block.position.substr(pos + 4) : "startpos";

      // Assign DEPTH
      std::string numberStr = cmd.substr(cmd.find_last_of(" ") + 1);
      int depth = std::stoi(numberStr);
      currentResult.depth = depth;

      // assign MOVES AND NUM_NODES
      std::ifstream fileResults(OUTPUT_FILE_SF);
      std::string line;
      if (!fileResults.is_open()) {
        std::cerr << "Error: Failed to open Stockfish output file for reading: " + OUTPUT_FILE_SF << std::endl;
        // Skip this result and continue to the next if unable to read results
        continue;
      }
      while (std::getline(fileResults, line)) {
        // If a line contains "Nodes searched", parse and set the number of nodes.
        if (line.find("Nodes searched") != std::string::npos) {
          std::istringstream iss(line);
          std::string move;
          std::string separator;  // for the colon
          int nodes;

          iss >> move >> separator >> nodes;
          currentResult.num_nodes = nodes;

        } else if (line.find(":") != std::string::npos) {
          // If a line contains a colon, it's assumed to be a move, parse it accordingly.
          currentResult.move_node.push_back(parseMoveAndNodes(line));
        }
      }
      // Add the current result to the results array and reset it for the next iteration.
      currentResult.engine = stockfish;
      results_array.push_back(currentResult);
      currentResult = PerftResult();

      // Close the output file.
      fileResults.close();
    }
  }

  return results_array;
}

/**
 * Parses a file containing Perft results generated by a chess engine and structures them into a
 * vector of PerftResult objects.
 *
 * @param filePath; Path to the file containing the Perft results to be parsed.
 * @return Vector of PerftResult objects.
 */
std::vector<PerftResult> parsePerftResultsLB(const std::string &filePath) {
  // Init an empty vector to store the Perft results.
  std::vector<PerftResult> results;
  // Init temp object to hold the current Perft result being processed.
  PerftResult currentResult;
  // Open file for reading and init a line read from file.
  std::ifstream file(filePath);
  std::string line;

  if (!file.is_open()) {
    std::cerr << "Failed to open file: " << filePath << std::endl;
    return results;
  }

  while (std::getline(file, line)) {
    if (line.find("position") != std::string::npos) {
      // If we're already processing a result, add it to the list
      if (!currentResult.move_node.empty()) {
        results.push_back(currentResult);
        currentResult = PerftResult();  // Reset for the next result
      }

      // Extract the position/FEN
      auto pos = line.find("fen");
      currentResult.fen = (pos != std::string::npos) ? line.substr(pos + 4) : "startpos";

    } else if (line.find("go depth") != std::string::npos) {
      std::istringstream iss(line);
      std::string temp;
      iss >> temp >> temp >> currentResult.depth;  // Extract depth
    } else if (line.find("Nodes") != std::string::npos) {
      std::istringstream iss(line);
      std::string move;
      std::string separator;  // for the colon
      int nodes;

      iss >> move >> separator >> nodes;
      currentResult.num_nodes = nodes;

      currentResult.engine = triglav;
      results.push_back(currentResult);

      currentResult = PerftResult();
      currentResult.fen = results.back().fen;

    } else if (line.find(':') != std::string::npos) {
      currentResult.move_node.push_back(parseMoveAndNodes(line));
    }
  }

  // Add the last result if it exists
  if (!currentResult.move_node.empty()) {
    results.push_back(currentResult);
  }

  return results;
}

/**
 * Compares the performance test results of two chess engines to identify differences.
 * This function checks for mismatches in the total number of nodes evaluated and
 * in individual move-node pairs for each position tested at a given depth.
 *  *
 * @param array1; Vector of PerftResult from the first engine.
 * @param array2; Vector of PerftResult from the second engine.
 * @return true; if no diff found else false.
 */
bool comparePerftResults(const std::vector<PerftResult> &array1, const std::vector<PerftResult> &array2) {
  // Init vectors for storing unique move_node pairs, when comparing
  std::vector<std::pair<std::string, int>> diff_1;
  std::vector<std::pair<std::string, int>> diff_2;
  // Flag to track if they are the same
  bool no_diff = true;

  for (const auto &result1 : array1) {
    bool matchFound = false;

    for (const auto &result2 : array2) {
      if (result1 == result2) {
        // When fen and depth match, check num_nodes and move_node vectors
        matchFound = true;

        // Check for a diff in the total number of nodes evaluated
        if (result1 < result2 || result1 > result2) {
          no_diff = false;
          //clang-format off
          std::cout << "----------------------------------------\n"
                    << "Mismatch in total number of nodes detected:\n"
                    << "Position (FEN): " << result1.fen << "\n"
                    << "Depth: " << result1.depth << "\n\n"
                    << "Mismatch Details:\n";
          std::cout << "- Engine: " << engines_str[result1.engine] << ": " << result1.num_nodes << "\n";
          std::cout << "- Engine: " << engines_str[result2.engine] << ": " << result2.num_nodes << "\n";
          //clang-format on
          std::cout << "----------------------------------------\n\n";
        }

        // Check for diff in move-node pairs
        for (auto const &i : result1.move_node) {
          if (std::find(std::begin(result2.move_node), std::end(result2.move_node), i) == std::end(result2.move_node)) {
            diff_1.push_back(i);
          }
        }
        for (auto const &i : result2.move_node) {
          if (std::find(std::begin(result1.move_node), std::end(result1.move_node), i) == std::end(result1.move_node)) {
            diff_2.push_back(i);
          }
        }
        // If diff in move-node pairs, print details
        if (!diff_1.empty() || !diff_2.empty()) {
          no_diff = false;
          //clang-format off
          std::cout << "----------------------------------------\n"
                    << "Mismatch in move:node pairs detected:\n"
                    << "Position (FEN): " << result1.fen << "\n"
                    << "Depth: " << result1.depth << "\n\n"
                    << "Mismatch Details:\n";
          std::cout << "- Engine: " << engines_str[result1.engine] << "\n";
          for (auto const &pair1 : diff_1) {
            std::cout << "  Move: " << pair1.first << ", Nodes: " << pair1.second << "\n\n";
          }
          std::cout << "- Engine: " << engines_str[result2.engine] << "\n";
          for (auto const &pair2 : diff_2) {
            std::cout << "  Move: " << pair2.first << ", Nodes: " << pair2.second << "\n\n";
          }
          std::cout << "----------------------------------------\n\n";

          //clang-format on
        }

        // Reset diff vectors
        diff_1.clear();
        diff_2.clear();
      }
    }
    // If no matching FEN and depth were found in array2 for a PerftResult in array1
    if (!matchFound) {
      std::cout << "No match found for FEN: " << result1.fen << " at depth " << result1.depth
                << " in the second engine." << std::endl;

      no_diff = false;
    }
  }
  return no_diff;
}

/**
 * Runs performance tests (Perft) against the Stockfish chess engine and compares the results
 * with this chess engine's Perft results.
 *
 * @param path_to_sf; String reference containing the path to the Stockfish executable.
 */
void ChessGame::testAgainstSF(std::string &path_to_sf) {
  namespace fs = std::filesystem;
  fs::path pathObj(path_to_sf);
  // Check if the path exists and is a regular file
  if (!fs::exists(pathObj) || !fs::is_regular_file(pathObj)) {
    std::cout << "Error: The path to the executable is not valid." << std::endl;
    return;
  }

  if (pathObj.extension() != ".exe") {
    std::cout << "Error: The specified file does not have a .exe extension." << std::endl;
    return;
  }

  // Start ANALYSIS
  std::cout << "Analysing ... (if there are some big depths it can take a while (forever))." << std ::endl;
  // Flag property of ChessGame, indicating that output will be directed to file not terminal
  file_output = true;

  // Ensure correct path format (double backslash)
  std::string path_to_stockfish = ensureCorrectPathFormat(path_to_sf);

  // Parse the command blocks (from commands.txt)
  std::vector<CommandsBlock> blocks = parseCommandsBlocks(COMMANDS_FILE);

  // Iterate over each commands block, executing the contained Perft tests.
  for (const auto &block : blocks) {
    // Append the current position to the output file.
    std::ofstream outputMine(OUTPUT_FILE_LB, std::ios::app);
    outputMine << "\n" + block.position;
    outputMine.close();
    // Parse the position and execute the go (depth) commands for Perft testing.
    parsePosition(block.position.c_str());
    for (const auto &depth : block.go_depth) {
      parseGo(depth.c_str());
    }
  }
  // Parse the Perft results from the output file into a results vector.
  auto results_sf = parsePerftResultsLB(OUTPUT_FILE_LB);
  // Remove the output file as it's no longer needed.
  std::remove(OUTPUT_FILE_LB.c_str());

  // Obtain the Perft results from the Stockfish engine for comparison
  auto results_lb = parsePerftResultsSF(blocks, path_to_stockfish);

  // DEBUG
  // results_lb[0].move_node.push_back(std::make_pair("f4f4", 999));
  // results_lb[0].num_nodes = 99;

  // PRINT results
  // printPerftResults(results_sf);
  // printPerftResults(results_lb);

  // Print success if there are no diff
  if (comparePerftResults(results_sf, results_lb)) {
    std::cout << "Success: All " << results_sf.size() << " Perft tests are consistent between engines "
              << engines_str[results_lb[0].engine] << " and " << engines_str[results_sf[0].engine] << std::endl;
  }

  file_output = false;
}