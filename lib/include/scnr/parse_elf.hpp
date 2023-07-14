#pragma once

#include <scnr/types.hpp>
#include <scnr/util.hpp>

#include <optional>
#include <ostream>

namespace scnr {

struct ElfFile {
  std::endian endian = {};
  bool w64 = false;
  std::string_view cputype;
  std::string interpreter;

  bool operator==(const ElfFile& rhs) const noexcept {
    return endian == rhs.endian && w64 == rhs.w64 && cputype == rhs.cputype && interpreter == rhs.interpreter;
  }

  friend std::ostream& operator<<(std::ostream& os, const ElfFile& file) {
    return os << "elf = ["
              << (file.endian == std::endian::little ? "little, "
                                                     : (file.endian == std::endian::big ? "big, " : "native, "))
              << file.cputype << ", " << (file.w64 ? "x64, " : "x32, ") << file.interpreter << "]";
  }
};

std::optional<ElfFile> try_elf(scnr::StreamData stream);

}  // namespace scnr

template <>
struct std::hash<scnr::ElfFile> {
  inline std::size_t operator()(const scnr::ElfFile& elf) const noexcept {
    std::uint64_t ret = 0;
    scnr::hash_combine(ret, std::hash<std::endian>{}(elf.endian));
    scnr::hash_combine(ret, std::hash<bool>{}(elf.w64));
    scnr::hash_combine(ret, std::hash<std::string_view>{}(elf.cputype));
    scnr::hash_combine(ret, std::hash<std::string>{}(elf.interpreter));
    return ret;
  }
};