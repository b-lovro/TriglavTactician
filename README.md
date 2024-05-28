<div align="center">

  <h1>TriglavTactician</h1>

  An evolving UCI chess engine for analysis and play.

</div>

# Overview

TriglavTactician is a chess engine written in C++. It operates using bitboard data structures to efficiently represent the chessboard, a technique favored by many chess software developers. At its current stage, TriglavTactician can process only the fundamental commands of the Universal Chess Interface (UCI) protocol. You can dive deeper into how to interact with TriglavTactician using these commands by visiting our [documentation][documentation_link].

TriglavTactician is a command-line program. This means it doesn't come with a graphical user interface (GUI) to display the chessboard or allow for interactive move making visually. But you can pair it with a third-party chess GUI. You can find them at the [bottom](#free-guis).

# Files

Included in the TriglavTactician distribution:

  * [README.md][readme_link] - Your guide to the TriglavTactician chess engine.
  * [DOCUMENTATION.md][documentation_link] - Your guide to the TriglavTactician chess engine.
  * [src][src_link] - Contains the source code.


# Compiling TriglavTactician

## Prerequisites

Before compiling TriglavTactician, ensure you have the following:
- A C++ compiler (such as GCC or Clang) installed on your system.
- Basic knowledge of terminal (Unix-like systems) or command prompt / PowerShell (Windows).

### Cloning the Repository

You can download the source code directly from GitHub:

* Navigate to the TriglavTactician GitHub page.
* Look for the Code button and click it.
* Click Download ZIP.
* Once downloaded, extract the ZIP file to your desired location.

After downloading, navigate to the downloaded directory to begin the setup process.

### Compiling 

Compile the source code with the following command:

```bash
g++ -std=c++17 -O3 -o TriglavTactician *.cpp
```

### Running TriglavTactician 

After compilation, you can run TriglavTactician directly from the terminal or command prompt:

##### On Unix-like Systems
```bash
./TriglavTactician
```

##### On Windows
```bash
TriglavTactician.exe
```

# Features

### Moves generation
* Using bitboards to represent pieces on the board
* Pre-calculated attacks for leaping pieces (pawn, king, knight)
* Sliding attacks and moves calculated on the fly
* Moves encoded as integers

### Decision making
* Using [Negamax][nega_link] search with alpa-beta prunning
* Move ordering
* [Killer Heuristic][kill_link] and [History Heuristic][his_link]
* [Principal Variation ][PV_link]
* [Quiescence Search][qs_link]

## Reference 
* Code Monkey King and his youtube channel [Chess Programming](https://www.youtube.com/channel/UCB9-prLkPwgvlKKqDgXhsMQ).
* Holy Grail for chess engines: [Chess Programming Wiki](https://www.chessprogramming.org/Main_Page)

## Free GUIs:

To get started, simply install one of the recommended GUIs and refer to their documentation for instructions on how to incorporate chess engine into the interface. Once set up, you're all set to test TriglavTactician against other chess engines or humans.

* [En Croissant](https://encroissant.org/)
* [Arena](http://www.playwitharena.de/)
* [BanksiaGUI](https://banksiagui.com/download/)












[readme_link]: https://github.com/b-lovro/TriglavTactician/blob/master/README.md
[documentation_link]:https://github.com/b-lovro/TriglavTactician/blob/master/DOCUMENTATION.md
[src_link]: https://github.com/b-lovro/TriglavTactician/tree/master/src
[qs_link]: https://www.chessprogramming.org/Quiescence_Search
[PV_link]: https://www.chessprogramming.org/Principal_Variation
[kill_link]: https://www.chessprogramming.org/Killer_Heuristic
[his_link]: https://www.chessprogramming.org/History_Heuristic
[nega_link]: https://www.chessprogramming.org/Negamax
[video_link]: https://cloud.uni-konstanz.de/index.php/s/5dLaXSPAq2b34p4