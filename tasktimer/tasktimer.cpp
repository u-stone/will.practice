#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>

class Timer {
public:
  Timer() : m_thread(), m_cv(), m_mtx(), m_interval(0), m_cancel(false) {}

  ~Timer() {
    if (m_thread.joinable()) {
      stop();
      m_thread.join();
    }
  }

  void start(int interval) {
    m_interval = interval;
    m_cancel = false;
    m_thread = std::thread(&Timer::run, this);
  }

  void stop() {
    m_cancel = true;
    m_cv.notify_all();
  }

  void setInterval(int interval) {
    m_interval = interval;
    m_cv.notify_all();
  }

private:
  void run() {
    std::unique_lock<std::mutex> lock(m_mtx);
    while (!m_cancel) {
      m_cv.wait_for(lock, std::chrono::milliseconds(m_interval));
      if (!m_cancel) {
        // do something
        std::cout << "Timer triggered" << std::endl;
      }
    }
  }

  std::thread m_thread;
  std::condition_variable m_cv;
  std::mutex m_mtx;
  int m_interval;
  bool m_cancel;
};

int main() {
  Timer timer;
  timer.start(1000);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  timer.setInterval(2000);
  std::this_thread::sleep_for(std::chrono::seconds(5));
  timer.stop();
  return 0;
}