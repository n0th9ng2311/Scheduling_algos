#pragma once
#include "CommonUtils.h"


class RoundRobinScheduler : public AbstractScheduler {
public:
  static constexpr double TIME_SLICE = 10.0;

  void sort(TaskQueue& task_queue) override {
    //RR preserves insertion order
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
  }

  bool runProcess(PID& task, TaskQueue& tq, double& currentTime) override {
    if (currentTime < task.getArrivalTime())
      currentTime = task.getArrivalTime();

    float slice = std::min(static_cast<float>(TIME_SLICE), task.getRemainingTime());
    task.decRemainingTime(slice);
    currentTime += slice;

    if (task.getRemainingTime() <= 0.0f) {
      task.calculateMetrics(static_cast<float>(currentTime));
      return true;
    }

    // no re-sort
    tq.addTask(std::move(task));
    return false;
  }

  [[nodiscard]] std::string name() const override { return "Round Robin"; }
};