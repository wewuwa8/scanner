#include <scnr/thread_pool.hpp>

namespace {
static thread_local scnr::ThreadPool* gPool = nullptr;
}

namespace scnr {

ThreadPool::ThreadPool(size_t workers) {
  for (size_t i = 0; i < workers; ++i) {
    workers_.emplace_back([&]() {
      auto was_pool = gPool;
      gPool = this;

      while (true) {
        auto task = task_queue_.Take();
        if (not task) {
          break;
        }

        try {
          task.value()();
        } catch (...) {
        }

        TaskDone();
      }

      gPool = was_pool;
    });
  }
}

ThreadPool::~ThreadPool() {
  assert(stopped_);
}

void ThreadPool::Submit(Task task) {
  tasks_.fetch_add(1);
  if (not task_queue_.Put(std::move(task))) {
    TaskDone();
  }
}

void ThreadPool::WaitIdle() {
  while (true) {
    auto tasks = tasks_.load();
    if (tasks == 0) {
      break;
    }
    tasks_.wait(tasks);
  }
}

void ThreadPool::Stop() {
  assert(not stopped_);

  task_queue_.Cancel();
  for (auto& worker : workers_) {
    worker.join();
  }

  stopped_ = true;
}

void ThreadPool::TaskDone() {
  if (tasks_.fetch_add(-1) == 1) {
    tasks_.notify_all();
  }
}

ThreadPool* ThreadPool::Current() {
  return gPool;
}

}  // namespace scnr
