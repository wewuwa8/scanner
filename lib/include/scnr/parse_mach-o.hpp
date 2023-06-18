#pragma once

#include <scnr/types.hpp>
#include <scnr/util.hpp>

#include <bit>
#include <cstdint>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>

namespace scnr {

struct MachOSingle {
  std::endian endian = {};
  bool w64 = false;
  std::string_view cputype;
  bool issigned = false;

  bool operator==(const MachOSingle& rhs) const noexcept {
    return endian == rhs.endian && w64 == rhs.w64 && cputype == rhs.cputype && issigned == rhs.issigned;
  }

  friend std::ostream& operator<<(std::ostream& os, const MachOSingle& file) {
    return os << "mach-o = ["
              << (file.endian == std::endian::little ? "little, "
                                                     : (file.endian == std::endian::big ? "big, " : "native, "))
              << file.cputype << ", " << (file.w64 ? "x64, " : "x32, ") << (file.issigned ? "signed]" : "unsigned]");
  }
};

struct MachOFat {
  std::vector<MachOSingle> files;

  bool operator==(const MachOFat& rhs) const noexcept {
    return files == rhs.files;
  }

  friend std::ostream& operator<<(std::ostream& os, const MachOFat& file) {
    os << "mach-o fat " << file.files.size() << " ";
    bool comma = false;
    for (const auto& single : file.files) {
      if (comma) {
        os << ", ";
      }
      comma = true;
      os << single;
    }
    return os;
  }
};

struct MachOFile {
  std::variant<MachOSingle, MachOFat> value;

  friend std::ostream& operator<<(std::ostream& os, const MachOFile& file) {
    std::visit(
      [&](auto&& arg) {
        os << arg;
      },
      file.value);
    return os;
  }

  bool operator==(const MachOFile& rhs) const noexcept {
    return value == rhs.value;
  }
};

std::optional<MachOFile> try_macho(const Byte* buf, size_t nbytes);

}  // namespace scnr

template <>
struct std::hash<scnr::MachOSingle> {
  inline std::size_t operator()(const scnr::MachOSingle& single) const noexcept {
    std::uint64_t ret = 0;
    scnr::hash_combine(ret, std::hash<std::endian>{}(single.endian));
    scnr::hash_combine(ret, std::hash<bool>{}(single.w64));
    scnr::hash_combine(ret, std::hash<std::string_view>{}(single.cputype));
    scnr::hash_combine(ret, std::hash<bool>{}(single.issigned));
    return ret;
  }
};

template <>
struct std::hash<scnr::MachOFat> {
  inline std::size_t operator()(const scnr::MachOFat& fat) const noexcept {
    std::uint64_t ret = 0;
    scnr::hash_combine(ret, std::hash<size_t>{}(fat.files.size()));
    for (const auto& file : fat.files) {
      scnr::hash_combine(ret, std::hash<scnr::MachOSingle>{}(file));
    }
    return ret;
  }
};

template <>
struct std::hash<scnr::MachOFile> {
  inline std::size_t operator()(const scnr::MachOFile& file) const noexcept {
    std::uint64_t ret = 0;
    std::visit(
      [&ret](auto&& arg) {
        ret = std::hash<std::decay_t<decltype(arg)>>{}(arg);
      },
      file.value);
    return ret;
  }
};