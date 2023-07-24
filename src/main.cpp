#include <scnr/context.hpp>
#include <scnr/scnr.hpp>
#include <scnr/thread_pool.hpp>

#include <csignal>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
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

struct CmdOptions {
  std::optional<int> jobs;
  std::vector<std::string> files;

  static constexpr std::string_view help_message = R"(Usage: scanner [OPTION...] FILE...
Determine type of FILEs and collect statistics
  -h, --help                  display this help and exit
  -j N, --jobs N              specifies the number of jobs (commands) to run simultaneously
)";

  void print_help() {
    std::cout << help_message;
    std::exit(1);
  }

  void parse(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
      auto arg = argv[i];
      if (std::strcmp(arg, "-h") == 0 || std::strcmp(arg, "--help") == 0) {
        print_help();
      }
      if (std::strcmp(arg, "-j") == 0 || std::strcmp(arg, "--jobs") == 0) {
        if (jobs.has_value() || !files.empty() || (i + 1 >= argc)) {
          print_help();
        }
        i += 1;
        try {
          jobs = std::atoi(argv[i]);
        } catch (...) {
          print_help();
        }
        if (jobs.value() <= 0) {
          print_help();
        }
      }
      files.push_back(arg);
    }

    if (files.empty()) {
      print_help();
    }
  }
};

int main(int argc, char** argv) {
  std::signal(SIGTERM, handle_stop);
  std::signal(SIGINT, handle_stop);

  CmdOptions options;
  options.parse(argc, argv);
  const int hw_concurrency = std::thread::hardware_concurrency();
  int jobs = options.jobs.value_or(hw_concurrency);
  jobs = std::min(hw_concurrency, jobs);

  scnr::ThreadPool pool(jobs, &scnr::gContext);
  scnr::FileInfoCollector collector;

  for (const auto& f : options.files) {
    pool.Submit([f, &collector]() {
      scnr::process(std::filesystem::path(f), collector);
    });
  }

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