#pragma once
#include <algorithm>
#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <future>
#include <iomanip>
#include <iostream>
#include <optional>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

using u8 = std::uint8_t;

enum class PSTATES : u8 {
  WAITING = 0,
  READY = 1,
  EXECUTING = 2,
};

static std::atomic<int> global_ID{0};

struct PID {
  char p_name{};
  int p_ID{};

  float p_remaining_time{};
  float p_burst_time{};
  float p_arrival_time{};
  float p_turnaround_time{};
  float p_completion_time{};
  float p_waiting_time{};

  uint8_t p_priority{};

  PSTATES p_status{PSTATES::WAITING};

  PID(char name, float arrival_time, float burst_time, uint8_t priority)
      : p_ID{global_ID++},
        p_name{name},
        p_arrival_time{arrival_time},
        p_burst_time{burst_time},
        p_remaining_time{burst_time},
        p_priority{priority} {}

  [[nodiscard]] int getPID() const { return p_ID; }
  [[nodiscard]] float getArrivalTime() const { return p_arrival_time; }
  [[nodiscard]] float getRemainingTime() const { return p_remaining_time; }
  [[nodiscard]] float getBurstTime() const { return p_burst_time; }
  [[nodiscard]] float getTurnaroundTime() const { return p_turnaround_time; }
  [[nodiscard]] float getWaitingTime() const { return p_waiting_time; }
  [[nodiscard]] float getCompletionTime() const { return p_completion_time; }
  [[nodiscard]] uint8_t getPriority() const { return p_priority; }

  void decRemainingTime(float dec) { p_remaining_time -= dec; }

  void calculateMetrics(float currentTime) {
    p_completion_time = currentTime;
    p_turnaround_time = p_completion_time - p_arrival_time;
    p_waiting_time = p_turnaround_time - p_burst_time;
  }

  friend std::ostream& operator<<(std::ostream& os, const PID& p) {
    os << std::left << "PID: " << std::setw(4) << p.p_ID << " | Name: " << std::setw(2) << p.p_name
       << " | Arr: " << std::fixed << std::setprecision(2) << std::setw(7) << p.p_arrival_time
       << " | Rem: " << std::setw(7) << p.p_remaining_time << " | Prio: " << std::setw(7)
       << static_cast<int>(p.p_priority) << "\n";
    return os;
  }
};

// The PID manager will be responsible for operations on individual PIDs not the storage or anything just the
// PID
class PID_Manager {
  public:
  static bool sortArrivalTime(const PID& left, const PID& right) {
    if (left.getArrivalTime() == right.getArrivalTime()) {
      return left.getPID() < right.getPID();
    }
    return left.getArrivalTime() < right.getArrivalTime();
  }

  static bool sortRemainingTime(const PID& left, const PID& right) {
    if (left.getRemainingTime() == right.getRemainingTime()) {
      return left.getPID() < right.getPID();
    }
    return left.getRemainingTime() < right.getRemainingTime();
  }

  // lower the prio number more the priority
  static bool sortPriority(const PID& left, const PID& right) {
    if (left.getPriority() == right.getPriority()) {
      return left.getPID() < right.getPID();
    }
    return left.getPriority() < right.getPriority();
  }
};

// Both waitQueue and taskQueue do share a lot in common, they will both inherit from this base abstractQueue
class AbstractQueue {
  public:
  AbstractQueue() = default;
  explicit AbstractQueue(std::deque<PID> other) : m_store{std::move(other)} {}
  virtual ~AbstractQueue() = default;

  [[nodiscard]] virtual bool empty() const {
    std::scoped_lock lk(m_mutex);
    return m_store.empty();
  }

  virtual void addTask(PID task) {
    std::scoped_lock lk(m_mutex);
    m_store.emplace_back(std::move(task));
  }

  virtual std::optional<PID> popTask() {
    std::scoped_lock lock{m_mutex};
    if (m_store.empty()) return std::nullopt;
    PID front = std::move(m_store.front());
    m_store.pop_front();
    return front;
  }

  void addTaskQueue(std::vector<PID> other) {
    std::scoped_lock lock{m_mutex};
    m_store.insert(
        m_store.end(), std::make_move_iterator(other.begin()), std::make_move_iterator(other.end()));
  }

  [[nodiscard]] std::size_t size() const {
    std::scoped_lock lk(m_mutex);
    return m_store.size();
  }

  void print() {
    std::scoped_lock lk{m_mutex};
    for (const auto& el : m_store) {
      std::cout << el;
    }
  }

  protected:
  std::deque<PID> m_store;
  mutable std::mutex m_mutex;  // mutable so const methods can lock
};

class waitQueue : public AbstractQueue {
  public:
  waitQueue() = default;
  explicit waitQueue(std::deque<PID> tasks) : AbstractQueue(std::move(tasks)) {}

  void sortArrival() {
    std::scoped_lock lk(m_mutex);
    std::sort(m_store.begin(), m_store.end(), PID_Manager::sortArrivalTime);
  }
};

class ReadyQueue : public AbstractQueue {
  public:
  ReadyQueue() = default;
  explicit ReadyQueue(std::deque<PID> tasks) : AbstractQueue(std::move(tasks)) {}
};

class TaskQueue : public AbstractQueue {
  public:
  TaskQueue() = default;
  explicit TaskQueue(std::deque<PID> other) : AbstractQueue(std::move(other)) {}

  void sortWith(std::function<bool(const PID&, const PID&)> cmp) {
    std::scoped_lock lk(m_mutex);
    std::sort(m_store.begin(), m_store.end(), cmp);
  }
};
// ques manager class will facilitate all the communication between different

class queuesManager {
  public:
  queuesManager() = delete;

  static void moveTaskWR(waitQueue& wq, ReadyQueue& rq, std::atomic<bool>& stop) {
    while (!stop.load()) {
      if (auto task = wq.popTask()) {
        task->p_status = PSTATES::READY;
        rq.addTask(std::move(*task));
      } else {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    }
  }

  static void moveTaskRT(ReadyQueue& rq, TaskQueue& tq, std::atomic<bool>& stop) {
    while (!stop.load()) {
      if (auto task = rq.popTask()) {
        task->p_status = PSTATES::EXECUTING;
        tq.addTask(std::move(*task));
      } else {
        std::this_thread::sleep_for(std::chrono::microseconds(100));
      }
    }
  }
};

// this class will be the base of all other schedulers and will have the following memebrs:
//  sort() -> reorders the members based on whatever strategy is chosen
//  name() -> gives the name of the scheduling chosen

class AbstractScheduler {
  public:
  virtual ~AbstractScheduler() = default;

  virtual void sort(TaskQueue& tq) = 0;

  // returns true if the process is done false if preempted
  virtual bool runProcess(PID& task, TaskQueue& tq, double& currentTime) = 0;

  [[nodiscard]] virtual std::string name() const = 0;
};

// Aggregate metrics from one scheduler run, used by the simulation test to
struct SimResult {
  std::string scheduler_name;
  double avg_turnaround{};
  double avg_waiting{};
  double avg_completion{};
  int process_count{};
};

// this will simulate our worker which will pick tasks and then work on them
class Worker {
  public:
  Worker(TaskQueue& queue, AbstractScheduler& scheduler) : m_queue{queue}, m_scheduler{scheduler} {}

  bool work(double& currentTime) {
    auto maybeTask = m_queue.popTask();
    if (!maybeTask) return false;

    PID task = std::move(*maybeTask);
    bool done = m_scheduler.runProcess(task, m_queue, currentTime);

    if (done) {
      std::cout << "Completed PID " << task.getPID() << " ('" << task.p_name << "')"
                << "  CT=" << std::fixed << std::setprecision(2) << task.getCompletionTime()
                << "  TAT=" << task.getTurnaroundTime() << "  WT=" << task.getWaitingTime() << "\n";
      m_completed.push_back(std::move(task));
    }
    return done;
  }

  void printSummary() const {
    if (m_completed.empty()) return;

    float totalTAT = 0, totalWT = 0;
    std::cout << "\n--- Final Summary (" << m_scheduler.name() << ") ---\n";
    std::cout << std::left << std::setw(6) << "PID" << std::setw(6) << "Name" << std::setw(10) << "Arrival"
              << std::setw(10) << "Burst" << std::setw(12) << "Completion" << std::setw(12) << "Turnaround"
              << std::setw(10) << "Waiting" << "\n";
    std::cout << std::string(66, '-') << "\n";

    for (const auto& p : m_completed) {
      totalTAT += p.getTurnaroundTime();
      totalWT += p.getWaitingTime();
      std::cout << std::fixed << std::setprecision(2) << std::setw(6) << p.getPID() << std::setw(6)
                << p.p_name << std::setw(10) << p.getArrivalTime() << std::setw(10) << p.getBurstTime()
                << std::setw(12) << p.getCompletionTime() << std::setw(12) << p.getTurnaroundTime()
                << std::setw(10) << p.getWaitingTime() << "\n";
    }

    auto n = static_cast<float>(m_completed.size());
    std::cout << "\nAvg Turnaround Time : " << totalTAT / n << "\n";
    std::cout << "Avg Waiting Time    : " << totalWT / n << "\n";
  }

  SimResult getSimResult(const std::string& sname) const {
    SimResult r;
    r.scheduler_name = sname;
    r.process_count = static_cast<int>(m_completed.size());
    if (m_completed.empty()) return r;

    double totalTAT = 0;
    double totalWT = 0;
    double totalCT = 0;

    for (const auto& p : m_completed) {
      totalTAT += p.getTurnaroundTime();
      totalWT += p.getWaitingTime();
      totalCT += p.getCompletionTime();
    }
    auto n = static_cast<float>(r.process_count);
    r.avg_turnaround = totalTAT / n;
    r.avg_waiting = totalWT / n;
    r.avg_completion = totalCT / n;
    return r;
  }

  private:
  TaskQueue& m_queue;
  AbstractScheduler& m_scheduler;
  std::vector<PID> m_completed;
};

// this class we create and push simulated work on our queue, these tasks will have
// 1) Random arrival time
// 2) Random burst time
// 3) Random task name
class RandomWork {
  public:
  RandomWork()
      : rng(rd()),
        char_dist(0, characters.size() - 1),
        arrival_dist(0.0f, 100.0f),
        burst_dist(10.0f, 200.0f),
        prio_dist(1, 100) {}

  char generateName() { return characters[char_dist(rng)]; }
  float generateArrivalTime() { return arrival_dist(rng); }
  float generateBurstTime() { return burst_dist(rng); }
  uint8_t generatePriority() { return static_cast<uint8_t>(prio_dist(rng)); }

  template <typename T>
  void pushTasksA(T& queue, int count) {
    std::vector<PID> temp_vec;
    temp_vec.reserve(count);
    for (int i = 0; i < count; ++i) {
      // Pass the new priority to the constructor
      PID temp{generateName(), generateArrivalTime(), generateBurstTime(), generatePriority()};
      temp_vec.emplace_back(std::move(temp));
    }
    queue.addTaskQueue(std::move(temp_vec));
  }

  private:
  const std::string characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  std::random_device rd;
  std::mt19937 rng;
  std::uniform_int_distribution<size_t> char_dist;
  std::uniform_real_distribution<float> arrival_dist;
  std::uniform_real_distribution<float> burst_dist;
  std::uniform_int_distribution<int> prio_dist;
};