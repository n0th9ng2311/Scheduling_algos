#pragma once

#include "CommonUtils.h"

class SRJFScheduler : public AbstractScheduler {
 public:
  static constexpr double TIME_SLICE = 10.0f;

  void sort(TaskQueue& tq) override {
    tq.sortWith([](const PID& a, const PID& b) {
      return PID_Manager::sortRemainingTime(a, b);
    });
  }

  // run one slice, re-queue if not done, re-sort so a shorter job
  // can jump ahead next turn. Returns true only when the process finishes.
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

    // Not done
    tq.addTask(std::move(task));
    sort(tq);
    return false;
  }

  [[nodiscard]] std::string name() const override {
    return "Shortest Remaining Time First (SRTF)";
  }
};