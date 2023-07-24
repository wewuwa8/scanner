#pragma once

#include <scnr/types.hpp>
#include <scnr/util.hpp>

#include <optional>
#include <ostream>
#include <string_view>

namespace scnr {

struct TxtFile {
  std::string_view encoding;
  bool withbom = false;

  bool operator==(const TxtFile& rhs) const noexcept {
    return withbom == rhs.withbom && encoding == rhs.encoding;
  }

  friend std::ostream& operator<<(std::ostream& os, const TxtFile& file) {
    return os << "txt = [" << file.encoding << (file.withbom ? " with bom]" : "]");
  }
};

std::optional<TxtFile> try_txt(scnr::StreamData stream);

}  // namespace scnr

template <>
struct std::hash<scnr::TxtFile> {
  std::size_t operator()(const scnr::TxtFile& txt) const noexcept {
    std::uint64_t ret = 0;
    scnr::hash_combine(ret, std::hash<std::string_view>{}(txt.encoding));
    scnr::hash_combine(ret, std::hash<bool>{}(txt.withbom));
    return ret;
  }
};