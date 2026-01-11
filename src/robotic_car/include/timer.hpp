#pragma once

#include <atomic>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <cstdint>
#include <iostream>
#include <mutex>
#include <ratio>
#include <thread>
#include <type_traits>
#include <utility>

class Timer {
  std::thread timer_thread_;
  std::atomic<bool> expired_{true};
  std::atomic<bool> is_running_{true};
  std::mutex mutex_;
  std::condition_variable cv_;
  std::chrono::steady_clock::time_point expire_time_;

public:
  Timer() {
    timer_thread_ = std::thread([this]() {
      while (is_running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        if (expired_) {
          cv_.wait(lock, [this]() { return !expired_ || !is_running_; });
          if (!is_running_) {
            break;
          }
          if (expired_) {
            continue;
          }
        } else {
          if (cv_.wait_until(lock, expire_time_,
                             [this]() { return !is_running_; })) {
            break;
          }
          if (!expired_) {
            expired_ = true;
          }
        }
      }
    });
  }

  ~Timer() {
    is_running_ = false;
    cv_.notify_all();
    if (timer_thread_.joinable()) {
      timer_thread_.join();
    }
  }

  Timer(const Timer &) = delete;
  Timer &operator=(const Timer &) = delete;
  Timer(Timer &&) = delete;
  Timer &operator=(Timer &&) = delete;

  void expire_after(std::chrono::milliseconds duration) {
    std::unique_lock<std::mutex> lock(mutex_);
    expired_ = false;
    expire_time_ = std::chrono::steady_clock::now() + duration;
    cv_.notify_all();
  }

  void expire_at(std::chrono::steady_clock::time_point time_point) {
    std::unique_lock<std::mutex> lock(mutex_);
    expired_ = false;
    expire_time_ = time_point;
    cv_.notify_all();
  }

  [[nodiscard]] bool is_expired() const { return expired_; }
};