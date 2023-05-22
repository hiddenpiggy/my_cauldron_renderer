#ifndef TIMER_HPP
#define TIMER_HPP
#include <chrono>
#include <stdexcept>
namespace hiddenpiggy {
class Timer {
public:
  void start() {
    m_starttime = std::chrono::high_resolution_clock::now();
    started = true;
  }

  auto getCurrentTime() {
    if (started) {
      m_time = std::chrono::high_resolution_clock::now();
      return m_time - m_starttime;
    } else {
      throw std::runtime_error("timer usage invalid!");
    }
  }

  void stop() { started = false; }

private:
  bool started = false;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_time;
  std::chrono::time_point<std::chrono::high_resolution_clock> m_starttime;
};
} // namespace hiddenpiggy
#endif