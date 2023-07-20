#pragma once

#include <scnr/blocking_queue.hpp>
#include <scnr/context.hpp>

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

namespace scnr {

using Task = std::function<void()>;

// Fixed-size pool of worker threads
class ThreadPool {
 public:
  explicit ThreadPool(size_t workers, const Context* ctx = nullptr);
  ~ThreadPool();

  ThreadPool(const ThreadPool&) = delete;
  ThreadPool& operator=(const ThreadPool&) = delete;

  // Schedules task for execution in one of the worker threads
  void Submit(Task task);

  // Waits until outstanding work count has reached zero
  void WaitIdle();

  // Stops the worker threads as soon as possible
  // Pending tasks will be discarded
  void Stop();

  // Locates current thread pool from worker thread
  static ThreadPool* Current();

 private:
  void TaskDone(int tasks = 1);
  void CancelTasks();

 private:
  const scnr::Context* ctx_ = nullptr;
  bool stopped_ = false;
  std::vector<std::thread> workers_;
  UnboundedBlockingQueue<Task> task_queue_;

  std::atomic<uint32_t> tasks_{0};
};

}  // namespace scnr
