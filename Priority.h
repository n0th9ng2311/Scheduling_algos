#pragma once
#include "CommonUtils.h"

class PriorityScheduler : public AbstractScheduler {
public:
  static constexpr double TIME_SLICE = 10.0;

  void sort(TaskQueue& task_queue) override {
    task_queue.sortWith([](const PID& a, const PID& b) {
      return PID_Manager::sortPriority(a, b);
    });
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

    // Preempt
    tq.addTask(std::move(task));
    sort(tq);
    return false;
  }

  [[nodiscard]] std::string name() const override { return "Priority (Preemptive)"; }
};