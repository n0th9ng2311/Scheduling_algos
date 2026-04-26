#pragma once

#include "CommonUtils.h"


class SJFScheduler : public AbstractScheduler {
public:
  void sort(TaskQueue& tq) override {
    tq.sortWith([](const PID& a, const PID& b) {
      return PID_Manager::sortRemainingTime(a, b);
    });
  }

  bool runProcess(PID& task, TaskQueue& , double& currentTime) override {
    // Advance clock if CPU was idle before this process arrived
    if (currentTime < task.getArrivalTime())
      currentTime = task.getArrivalTime();


    currentTime += task.getRemainingTime();
    task.decRemainingTime(task.getRemainingTime());
    task.calculateMetrics(static_cast<float>(currentTime));
    return true;  //non-preemptive never returns false
  }

  [[nodiscard]] std::string name() const override { return "Shortest Job First (SJF)"; }
};