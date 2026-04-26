#pragma once
#include "FCFS.h"
#include "Priority.h"
#include "RR.h"
#include "SJF.h"
#include "SRJF.h"

inline void schedulerC(TaskQueue& Tqueue, AbstractScheduler& scheduler) {
  std::cout << "\nUsing scheduler: " << scheduler.name() << "\n";

  auto sort_future = std::async(std::launch::async, [&] { scheduler.sort(Tqueue); });
  while (sort_future.wait_for(std::chrono::milliseconds(250)) != std::future_status::ready)
    std::cout << "Sorting...\n";

  std::cout << "\n=== TaskQueue after initial " << scheduler.name() << " sort ===\n";
  Tqueue.print();
  std::cout << "\n=== Processing ===\n";

  Worker worker{Tqueue, scheduler};
  double currentTime = 0.0;

  while (!Tqueue.empty()) worker.work(currentTime);

  worker.printSummary();
}

inline void start(int choice) {
  waitQueue Wqueue{};
  ReadyQueue Rqueue{};
  TaskQueue Tqueue{};

  RandomWork work{};
  work.pushTasksA(Wqueue, 30);

  std::cout << "=== Processes in Wait Queue (unsorted) ===\n";
  Wqueue.print();

  Wqueue.sortArrival();
  std::cout << "\n=== Wait Queue sorted by arrival time ===\n";
  Wqueue.print();

  std::atomic<bool> stopPipeline{false};

  std::thread t_wr(queuesManager::moveTaskWR, std::ref(Wqueue), std::ref(Rqueue), std::ref(stopPipeline));
  std::thread t_rt(queuesManager::moveTaskRT, std::ref(Rqueue), std::ref(Tqueue), std::ref(stopPipeline));

  std::cout << "\nWaiting for pipeline to fill TaskQueue...\n";
  while (Tqueue.size() < 30) std::this_thread::sleep_for(std::chrono::milliseconds(5));

  stopPipeline.store(true);
  t_wr.join();
  t_rt.join();

  switch (choice) {
    case 1: {
      FCFS scheduler{};
      schedulerC(Tqueue, scheduler);
      break;
    }
    case 2: {
      RoundRobinScheduler scheduler{};
      schedulerC(Tqueue, scheduler);
      break;
    }
    case 3: {
      SJFScheduler scheduler{};
      schedulerC(Tqueue, scheduler);
      break;
    }
    case 4: {
      SRJFScheduler scheduler{}; //need to implement this
      schedulerC(Tqueue, scheduler);
      break;
    }
    case 5: {
      PriorityScheduler scheduler{};
      schedulerC(Tqueue, scheduler);
      break;
    }
    default:
      FCFS scheduler{};
      schedulerC(Tqueue, scheduler);
  }
}