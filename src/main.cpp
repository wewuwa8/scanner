#include <scnr/context.hpp>
#include <scnr/scnr.hpp>
#include <scnr/thread_pool.hpp>

#include <csignal>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int stopcalls = 0;
void handle_stop(int sig) {
  stopcalls += 1;
  scnr::gContext.RequestStop();
  // Circuit breaker
  if (stopcalls >= 5) {
    std::abort();
  }
}

int main(int argc, char** argv) {
  if (argc != 2 || std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0) {
    std::cout << "Usage: -h, --help\t\t\tprint this\n" << argv[0] << " [file or directory]\n";
    return 1;
  }

  std::signal(SIGTERM, handle_stop);
  std::signal(SIGINT, handle_stop);

  std::filesystem::path path(argv[1]);
  scnr::ThreadPool pool(std::thread::hardware_concurrency(), &scnr::gContext);
  scnr::FileInfoCollector collector;
  pool.Submit([path, &collector]() {
    scnr::process(path, collector);
  });

  pool.WaitIdle();
  pool.Stop();

  if (scnr::gContext.StopRequested()) {
    std::cout << "\nInterrupted.\n";
  }
  for (const auto& [k, v] : collector.Summarize()) {
    std::cout << k << " - " << v << "\n";
  }

  if (scnr::gContext.StopRequested()) {
    return 1;
  }
  return 0;
}