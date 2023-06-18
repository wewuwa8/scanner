#pragma once

#include <condition_variable>
#include <deque>
#include <mutex>
#include <optional>

namespace scnr {

template <typename T>
class UnboundedBlockingQueue {
 public:
  bool Put(T value) {
    std::lock_guard lock(mutex_);
    if (closed_) {
      return false;
    }
    buffer_.push_back(std::move(value));
    cv_data_avail_.notify_one();
    return true;
  }

  std::optional<T> Take() {
    std::unique_lock lock(mutex_);
    while (buffer_.empty() && not closed_) {
      cv_data_avail_.wait(lock);
    }

    if (buffer_.empty()) {
      return std::nullopt;
    }
    auto retval = std::move(buffer_.front());
    buffer_.pop_front();
    return std::move(retval);
  }

  void Close() {
    CloseImpl(/*clear=*/false);
  }

  void Cancel() {
    CloseImpl(/*clear=*/true);
  }

 private:
  void CloseImpl(bool clear) {
    std::lock_guard lock(mutex_);
    closed_ = true;
    if (clear) {
      buffer_.clear();
    }
    cv_data_avail_.notify_all();
  }

 private:
  bool closed_ = false;
  std::mutex mutex_;
  std::condition_variable cv_data_avail_;
  std::deque<T> buffer_;
};

}  // namespace scnr