#pragma once
#include "CommonUtils.h"

class RoundRobinScheduler : public AbstractScheduler {
  public:
  void sort(TaskQueue& task_queue) override {
    // we dont need to do anything because of the reinsertion logic of "m_queue.addTask(std::move(task));"
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }

  [[nodiscard]] std::string name() const override { return "RoundRobin"; }
};