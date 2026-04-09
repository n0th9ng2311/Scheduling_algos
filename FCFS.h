#pragma once
#include "CommonUtils.h"

class FCFS : public AbstractScheduler {
  public:
  void sort(TaskQueue& task_queue) override {
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    task_queue.sortWith([](const PID& a, const PID& b) { return PID_Manager::sortArrivalTime(a, b); });
  }

  [[nodiscard]] std::string name() const override { return "FCFS"; }
};