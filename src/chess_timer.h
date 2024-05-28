#ifndef CHESS_TIMER_H_
#define CHESS_TIMER_H_
#include <chrono>
#include <iostream>

class Timer {
  long long thinking_time_ms = 0;
  std::chrono::high_resolution_clock::time_point start_point;

  // use 1/20 of the remaining time
  static constexpr long long THINKING_TIME_RATIO = 20;

 public:
  void StartTimer(long long remaining_time_ms, long long new_increment_time_ms);
  bool IsTimeOut();

  static constexpr long long DEFAULT_THINKING_TIME_MS = 2147483647;
  static constexpr long long DEFAULT_INCREMENT_TIME_MS = 0;
};

#endif  // CHESS_GAME_H_