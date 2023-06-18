#include <scnr/scnr.hpp>
#include <scnr/thread_pool.hpp>

#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
  if (argc != 2 || std::strcmp(argv[1], "-h") == 0 || std::strcmp(argv[1], "--help") == 0) {
    std::cout << "Usage: -h, --help\t\t\tprint this\n" << argv[0] << " [file or directory]\n";
    return 1;
  }

  std::filesystem::path path(argv[1]);
  scnr::ThreadPool pool(std::thread::hardware_concurrency());
  scnr::FileInfoCollector collector;
  pool.Submit([path, &collector]() {
    scnr::process(path, collector);
  });

  pool.WaitIdle();
  pool.Stop();

  for (const auto& [k, v] : collector.Summarize()) {
    std::cout << k << " - " << v << "\n";
  }

  return 0;
}