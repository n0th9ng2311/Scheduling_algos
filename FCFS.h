#pragma once
#include "CommonUtils.h"

class FCFS : public AbstractScheduler {
public:
  void sort(TaskQueue& task_queue) override {
    std::this_thread::sleep_for(std::chrono::milliseconds(15));
    task_queue.sortWith([](const PID& a, const PID& b) {
      return PID_Manager::sortArrivalTime(a, b);
    });
  }

  // Non-preemptive
  bool runProcess(PID& task, TaskQueue& , double& currentTime) override {
    if (currentTime < task.getArrivalTime())
      currentTime = task.getArrivalTime();

    currentTime += task.getRemainingTime();
    task.decRemainingTime(task.getRemainingTime());
    task.calculateMetrics(currentTime);
    return true;
  }

  [[nodiscard]] std::string name() const override { return "FCFS"; }
};