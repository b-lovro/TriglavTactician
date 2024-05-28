# TriglavTactician Documentation

Here you'll find guide on how to interact with TriglavTactician using the Universal Chess Interface (UCI) protocol and run automated tests against the Stockfish engine.

## Table of Contents

- [TriglavTactician Documentation](#triglavtactician-documentation)
  - [Table of Contents](#table-of-contents)
  - [Available Commands](#available-commands)
  - [UCI Protocol](#uci-protocol)
    - [Starting UCI Mode](#starting-uci-mode)
    - [New Game](#new-game)
    - [Set Position](#set-position)
    - [Start Calculating](#start-calculating)
    - [Best Move](#best-move)
    - [Print](#print)
    - [Quit](#quit)
  - [Starting a New Game:](#starting-a-new-game)
    - [Making a Move:](#making-a-move)
    - [Print the Board:](#print-the-board)
    - [Requesting Help:](#requesting-help)
    - [Quitting the Game:](#quitting-the-game)
    - [Turns:](#turns)
    - [Setting Up:](#setting-up)
  - [Running Tests Against Stockfish](#running-tests-against-stockfish)


## Available Commands

- `uci`: Initiates Universal Chess Interface mode, making the engine ready to accept UCI commands.
- `playgame`: Play a text based game against the machine.
- `test [path_to_stockfish_executable]`: Run a series of automated tests against the Stockfish engine.
- `help`: Displays available commands and their descriptions.
- `exit`: Exits the application.

<sup><sup> 
Replace `[path_to_stockfish_executable]` with the actual file path to your Stockfish engine executable when using the `test` command.
 </sup></sup>


## UCI Protocol

The UCI protocol facilitates communication between chess engines like TriglavTactician and GUIs, enabling game analysis, playing against the engine, or using the engine in engine versus engine matches. Below are the basic commands to interact with TriglavTactician UCI:

More about [UCI Protocol](https://gist.github.com/DOBRO/2592c6dad754ba67e6dcaec8c90165bf)
### Starting UCI Mode

- **Command**: `uci`
  Initiates UCI mode, identifying the engine and author, followed by sending 'uciok' to indicate readiness.

### New Game

- **Command**: `ucinewgame`
  Resets the engine's internal state for a new game.

### Set Position

- **Command**: `position [startpos | fen fenstring] [moves move1 move2 ...]`
  Sets the current board position using either 'startpos' for the standard game start or 'fen' followed by a FEN string for a specific position. Moves can be specified from the current position using 'moves'.

  ```plaintext
    Example:
  > position startpos moves a2a4 b7b6 
  > print
    8  r n b q k b n r
    7  p . p p p p p p
    6  . p . . . . . .
    5  . . . . . . . .
    4  P . . . . . . .
    3  . . . . . . . .
    2  . P P P P P P P
    1  R N B Q K B N R

       a b c d e f g h 

    Color:     white
    Enpassant:   --
    Castling:  KQkq

  > position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -  
  > print
      8  r . . . k . . r
      7  p . p p q p b .
      6  b n . . p n p .
      5  . . . P N . . .
      4  . p . . P . . .
      3  . . N . . Q . p
      2  P P P B B P P P
      1  R . . . K . . R

         a b c d e f g h 

        Color:     white
        Enpassant:   --
        Castling:  KQkq
    ```

### Start Calculating

- **Command**: `go`
  Instructs the engine to start calculating from the current position. 
    - You can limit the search depth with 'depth'. For example, `go depth 5` restricts the search to 5 moves deep.
    - You can limit the search time with 'time'. For example, `go movetime 5000` restricts the search time to 5000 miliseconds.

- **Perft Analysis**: `go perft [depth]`
  Command outputs the number of possible positions reached for each legal move from a given position, up to a specified depth. The summary includes the total depth tested, the number of nodes (positions) evaluated, and the time taken for the test. 

  ```plaintext
    Example:
    > position startpos 
    > go perft 6
      Perft performance test

        a2a3: 4463267
        a2a4: 5363555
        b2b3: 5310358
        b2b4: 5293555
        c2c3: 5417640
        c2c4: 5866666
        d2d3: 8073082
        d2d4: 8879566
        e2e3: 9726018
        e2e4: 9771632
        f2f3: 4404141
        f2f4: 4890429
        g2g3: 5346260
        g2g4: 5239875
        h2h3: 4463070
        h2h4: 5385554
        b1a3: 4856835
        b1c3: 5708064
        g1f3: 5723523
        g1h3: 4877234

              Depth: 6
        Nodes searched: 119060324
                  Time: 9283 ms
   
    ```

### Best Move

- When the engine concludes its calculations (after `go depth` or `go movetime` command), it outputs:
  - Informations about the search: `info score cp [score in centipawns] depth [how deep was the search] nodes [numebr of nodes searched] pv [move1 move2 move3 ... Principal Variation moves]`
  -  the best move with `bestmove [move]`, where `[move]` follows UCI move notation.


### Print

- **Command**: `print`
  Prints current chess board.

### Quit

- **Command**: `quit`
  Exits the UCI protocol.

**Tips**:
- Familiarize yourself with UCI move notation for efficient interaction.
- Utilize a compatible GUI for a more accessible interface with TriglavTactician, which can be found [here](README.md#free-guis).

## Starting a New Game:

- **Command:** `newgame`
  - Resets the game to the initial chess position.

### Making a Move:

- **Command:** Enter your move in standard chess notation (e.g., `e2e4` to move a pawn to e4).

### Print the Board:

- **Command:** `print`
  - Displays the current state of the chessboard.

### Requesting Help:

- **Command:** `help`
  - Displays available commands and their descriptions.

### Quitting the Game:

- **Command:** `quit`
  - Exits the game.

### Turns:

- Your move is requested after the prompt "Your turn:".
- After your move, TriglavTactician will take its turn, calculating the best move based on the depth or time you've set at the beginning.

### Setting Up:

- At the start, you'll choose whether to play as black or white and set the engine's depth or time limit for making moves.

## Running Tests Against Stockfish

TriglavTactician allows you to run a series of automated perft tests against the Stockfish engine. Perft tests are used to verify the accuracy of a chess engine's move generation function by counting all the possible legal moves up to a certain depth, ensuring that the engine correctly calculates all potential game states without missing or adding extra moves.
To utilize this feature:

1. Make sure the `test` subfolder in your engine's directory contains a file named `commands.txt`. This file should list all chess positions and moves you wish to test, formatted according to the UCI protocol.

2. Within `commands.txt`, each new test position must be preceded by the word "NEXT" on its own line, serving as a marker to separate test cases.

    ```plaintext
    Example:
    NEXT
    position startpos 
    go perft 3
    go perft 4
    NEXT
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -  
    go perft 3
    go perft 5
    ```
3. If all perft test results are the same the program will output:

    ```plaintext
    Success: All [num. of tests] Perft tests are consistent between engines StockFish  and TriglavTactician
    ```

4. If issues arise during test execution, try clearing the `test` subfolder of all files except for `commands.txt`. This can help resolve problems related to residual data from previous tests.




