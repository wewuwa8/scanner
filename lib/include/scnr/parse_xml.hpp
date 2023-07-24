#pragma once

#include <scnr/types.hpp>
#include <scnr/util.hpp>

#include <optional>
#include <ostream>
#include <string_view>

namespace scnr {

struct XmlFile {
  std::string_view encoding;

  bool operator==(const XmlFile& rhs) const noexcept {
    return encoding == rhs.encoding;
  }

  friend std::ostream& operator<<(std::ostream& os, const XmlFile& file) {
    return os << "xml = [" << file.encoding << "]";
  }
};

std::optional<XmlFile> try_xml(scnr::StreamData stream);

}  // namespace scnr

template <>
struct std::hash<scnr::XmlFile> {
  inline std::size_t operator()(const scnr::XmlFile& xml) const noexcept {
    return std::hash<std::string_view>{}(xml.encoding);
  }
};
