#include <scnr/parse_encoding.hpp>
#include <scnr/types.hpp>

namespace {

constexpr unsigned char kBell = 7;
constexpr unsigned char kBS = 8;
constexpr unsigned char kHT = 9;
constexpr unsigned char kLF = 10;
constexpr unsigned char kVT = 11;
constexpr unsigned char kFF = 12;
constexpr unsigned char kCR = 13;
constexpr unsigned char kEsc = 27;

/*
 * printable ascii or bell,
 * backspace, tab, line feed, form feed, carriage return, esc, nextline.
 */
bool looks_ascii(const scnr::Byte* buf, size_t nbytes) {
  for (size_t i = 0; i < nbytes; ++i) {
    const auto c = buf[i];
    bool ok = (32 <= c && c < 127) || (c == kBS) || (c == kHT) || (c == kLF) || (c == kVT) || (c == kFF) || (c == kCR);
    if (ok) {
      continue;
    }
    return false;
  }
  return true;
}

bool looks_utf8(const scnr::Byte* buf, size_t nbytes) {
  for (size_t i = 0; i < nbytes; ++i) {
    int ones = std::countl_one(buf[i]);
    if (ones == 0) {
      // 0xxxxxxx is plain ASCII
      continue;
    }
    if (ones == 1 || ones > 4) {
      // invalid UTF-8
      return 0;
    }
    ones -= 1;
    if (i + ones >= nbytes) {
      // unexpected eof
      return 0;
    }
    while (ones) {
      i += 1;
      if (std::countl_one(buf[i]) != 1) {
        return 0;
      }
      ones -= 1;
    }
  }
  return true;
}

bool looks_utf8_with_BOM(const scnr::Byte* buf, size_t nbytes) {
  if (nbytes > 3 && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf) {
    return looks_utf8(buf + 3, nbytes - 3);
  }
  return 0;
}

bool looks_utf32(const scnr::Byte* buf, size_t nbytes, bool bigendian) {
  if (nbytes % 4 != 0) {
    return false;
  }
  for (int i = 0; i < nbytes; i += 4) {
    uint64_t code = 0;
    if (bigendian) {
      code = buf[i + 0] << 0x18 | buf[i + 1] << 0x10 | buf[i + 2] << 0x8 | buf[i + 3] << 0x0;
    } else {
      code = buf[i + 0] << 0x0 | buf[i + 1] << 0x8 | buf[i + 2] << 0x10 | buf[i + 3] << 0x18;
    }
    // Unicode v15 contains 149186 characters and this number is growing.
    // Let's check this code in some reasonable limit.
    if (code > 150000) {
      return false;
    }
  }

  return true;
}

bool looks_utf32_with_BOM(const scnr::Byte* buf, size_t nbytes, bool bigendian) {
  if (nbytes < 4 || nbytes % 4 != 0) {
    return false;
  }
  if (bigendian && buf[0] == 0 && buf[1] == 0 && buf[2] == 0xfe && buf[3] == 0xff &&
      looks_utf32(buf + 4, nbytes - 4, bigendian)) {
    return true;
  }
  if (not bigendian && buf[0] == 0xff && buf[1] == 0xfe && buf[2] == 0 && buf[3] == 0 &&
      looks_utf32(buf + 4, nbytes - 4, bigendian)) {
    return true;
  }
  return false;
}

}  // namespace

namespace scnr {

std::optional<TxtFile> try_txt(const Byte* buf, size_t nbytes) {
  TxtFile txtfile;
  if (looks_ascii(buf, nbytes)) {
    txtfile.encoding = "ASCII";
  } else if (looks_utf8_with_BOM(buf, nbytes)) {
    txtfile.encoding = "UTF-8";
    txtfile.withbom = true;
  } else if (looks_utf8(buf, nbytes)) {
    txtfile.encoding = "UTF-8";
  } else if (looks_utf32_with_BOM(buf, nbytes, true)) {
    txtfile.encoding = "UTF-32-BE";
    txtfile.withbom = true;
  } else if (looks_utf32_with_BOM(buf, nbytes, false)) {
    txtfile.encoding = "UTF-32-LE";
    txtfile.withbom = true;
  } else if (looks_utf32(buf, nbytes, true)) {
    txtfile.encoding = "UTF-32-BE";
  } else if (looks_utf32(buf, nbytes, false)) {
    txtfile.encoding = "UTF-8-LE";
  } else {
    return {};
  }
  return txtfile;
}

}  // namespace scnr