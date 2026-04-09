#pragma once
#include "CommonUtils.h"

class PriorityScheduler : public AbstractScheduler {
  public:
  void sort(TaskQueue& task_queue) override {
    task_queue.sortWith([](const PID& a, const PID& b) { return PID_Manager::sortPriority(a, b); });
  }

  [[nodiscard]] std::string name() const override { return "PriorityScheduler"; }
};