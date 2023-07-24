#include <scnr/parse_encoding.hpp>
#include <scnr/parse_xml.hpp>

namespace scnr {

std::optional<XmlFile> try_xml(scnr::StreamData stream) {
  // poor man's detector, but 'file' utility works in the same way
  static constexpr std::string_view magic = "<?xml";
  Byte buf[magic.size()];
  if (!stream.read(buf, 0, magic.size())) {
    return {};
  }
  for (size_t i = 0; i < magic.size(); ++i) {
    if (magic[i] != buf[i]) {
      return {};
    }
  }

  auto txt = scnr::try_txt(stream.advanced(magic.size()));
  if (!txt) {
    return {};
  }
  return XmlFile{.encoding = txt.value().encoding};
}

}  // namespace scnr