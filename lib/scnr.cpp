#include <scnr/parse_elf.hpp>
#include <scnr/parse_encoding.hpp>
#include <scnr/parse_mach-o.hpp>
#include <scnr/scnr.hpp>
#include <scnr/thread_pool.hpp>
#include <scnr/util.hpp>

#include <algorithm>
#include <bit>
#include <cstddef>
#include <cstring>
#include <fstream>
#include <iostream>
#include <iterator>
#include <mutex>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <variant>

namespace {

void process_file_impl(const std::filesystem::path& path, scnr::FileInfoCollector& collector) {
  auto file = scnr::read_file(path);
  auto fileinfo = scnr::detect_content(file);
  collector.Add(std::move(fileinfo));
}

void process_impl(const std::filesystem::path& path, scnr::FileInfoCollector& collector) {
  auto thread_pool = scnr::ThreadPool::Current();

  if (std::filesystem::is_directory(path)) {
    std::filesystem::directory_iterator dir_iter(path);
    for (auto const& dir_entry : dir_iter) {
      auto path = dir_entry.path();
      if (thread_pool) {  // concurrent
        thread_pool->Submit([path, &collector] {
          process_impl(path, collector);
        });
      } else {  // straight recursive
        process_impl(path, collector);
      }
    }
    return;
  }

  if (std::filesystem::is_regular_file(path)) {
    process_file_impl(path, collector);
    return;
  }

  // symlinks?..
}

}  // namespace

namespace scnr {

FileInfo detect_content(scnr::StreamData stream) {
  FileInfo fileinfo;
  if (auto elf = try_elf(stream)) {
    fileinfo = std::move(elf.value());
  } else if (auto macho = try_macho(stream)) {
    fileinfo = std::move(macho.value());
  } else if (auto txt = try_txt(stream)) {
    fileinfo = std::move(txt.value());
  }
  return fileinfo;
}

void process(const std::filesystem::path& path, FileInfoCollector& collector) {
  process_impl(path, collector);
}

void FileInfoCollector::Add(const FileInfo& fileinfo) {
  std::lock_guard lock(mutex);
  mp[fileinfo] += 1;
}

std::vector<std::pair<int, FileInfo>> FileInfoCollector::Summarize() const {
  std::unique_lock lock(mutex);
  std::vector<std::pair<int, FileInfo>> retval;
  for (const auto& [k, v] : mp) {
    retval.push_back({v, k});
  }
  lock.unlock();
  std::sort(retval.begin(), retval.end(), [](const auto& lhs, const auto& rhs) {
    // sort by frequency, if equal by variant index (without any reason)
    if (lhs.first == rhs.first) {
      return lhs.second.index() < rhs.second.index();
    }
    return lhs.first > rhs.first;
  });
  return retval;
}

}  // namespace scnr