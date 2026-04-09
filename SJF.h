#pragma once

#include "CommonUtils.h"

// Tie-breaking: lower PID wins
class SJFScheduler : public AbstractScheduler {
  public:
  void sort(TaskQueue& task_queue) override {
    task_queue.sortWith([](const PID& a, const PID& b) {
      // Simulate scheduling overhead
      std::this_thread::sleep_for(std::chrono::milliseconds(15));
      return PID_Manager::sortRemainingTime(a, b);
    });
  }

  [[nodiscard]] std::string name() const override { return "Shortest Job First (SJF)"; }
};
