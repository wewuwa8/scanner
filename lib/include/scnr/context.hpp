#pragma once

#include <atomic>

namespace scnr {

class Context {
 public:
  void RequestStop() noexcept {
    stop_requested_.test_and_set();
  }

  bool StopRequested() const noexcept {
    return stop_requested_.test();
  }

 private:
  std::atomic_flag stop_requested_ = ATOMIC_FLAG_INIT;
};

extern Context gContext;

}  // namespace  scnr
