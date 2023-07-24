#pragma once

#include <scnr/file.hpp>
#include <scnr/types.hpp>
#include <scnr/util.hpp>

#include <bit>
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>

namespace scnr {

struct PEFile {
  std::endian endian = {};
  bool w64 = false;
  std::string_view cputype;
  bool managed = false;

  bool operator==(const PEFile& rhs) const noexcept {
    return endian == rhs.endian && w64 == rhs.w64 && cputype == rhs.cputype && managed == rhs.managed;
  }

  friend std::ostream& operator<<(std::ostream& os, const PEFile& file) {
    return os << "PE = ["
              << (file.endian == std::endian::little ? "little, "
                                                     : (file.endian == std::endian::big ? "big, " : "native, "))
              << file.cputype << ", " << (file.w64 ? "x64, " : "x32, ") << (file.managed ? "managed]" : "]");
  }
};

std::optional<PEFile> try_pe(scnr::StreamData stream);

}  // namespace scnr

template <>
struct std::hash<scnr::PEFile> {
  inline std::size_t operator()(const scnr::PEFile& pe) const noexcept {
    std::uint64_t ret = 0;
    scnr::hash_combine(ret, std::hash<std::endian>{}(pe.endian));
    scnr::hash_combine(ret, std::hash<bool>{}(pe.w64));
    scnr::hash_combine(ret, std::hash<std::string_view>{}(pe.cputype));
    scnr::hash_combine(ret, std::hash<bool>{}(pe.managed));
    return ret;
  }
};
