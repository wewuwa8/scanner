#pragma once

#include <scnr/parse_elf.hpp>
#include <scnr/parse_encoding.hpp>
#include <scnr/parse_mach-o.hpp>
#include <scnr/parse_pe.hpp>
#include <scnr/parse_xml.hpp>
#include <scnr/types.hpp>

#include <mutex>
#include <unordered_map>
#include <utility>
#include <variant>

namespace scnr {

using FileInfo = std::variant<std::monostate, ElfFile, MachOFile, PEFile, TxtFile, XmlFile>;

class FileInfoCollector {
 public:
  void Add(const FileInfo& fileinfo);
  std::vector<std::pair<int, FileInfo>> Summarize() const;

 private:
  mutable std::mutex mutex;
  std::unordered_map<FileInfo, int> mp;
};

FileInfo detect_content(scnr::StreamData stream);
void process(const std::filesystem::path& path, FileInfoCollector& collector);

}  // namespace scnr

inline std::ostream& operator<<(std::ostream& os, const scnr::FileInfo& info) {
  std::visit(
    [&](auto&& arg) {
      if constexpr (not std::is_same_v<std::decay_t<decltype(arg)>, std::monostate>) {
        os << arg;
      } else {
        os << "Unknown";
      }
    },
    info);
  return os;
}