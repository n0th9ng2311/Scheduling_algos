#pragma once
#include "CommonUtils.h"

class SRJFscheduler : public AbstractScheduler {
  public:
  void sort(TaskQueue& task_queue) override {}

  [[nodiscard]] std::string name() const override { return "Shortest Remaining Job First Scheduler"; }
};