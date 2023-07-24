#include <scnr/parse_encoding.hpp>
#include <scnr/types.hpp>

namespace {

// Printable chars, Bell, Backspace, HT, LineFeed, VT, FormFeed, CR, NEL
bool looks_ascii(scnr::Byte c) {
  return (32 <= c && c < 127) || (7 <= c && c < 14) || c == 133;
}

bool looks_iso8859_1(scnr::Byte c) {
  return looks_ascii(c) || (160 <= c);
}

bool looks_extended_ascii(scnr::Byte c) {
  return looks_ascii(c) || 128 <= c;
}

template <typename Func>
bool looks(scnr::StreamData stream, Func&& func) {
  scnr::Byte buf[1024];
  while (auto nbytes = stream.readnext(buf, sizeof(buf))) {
    for (size_t i = 0; i < nbytes; ++i) {
      if (!func(buf[i])) {
        return false;
      }
    }
  }
  return true;
}

bool looks_ascii(scnr::StreamData stream) {
  return looks(stream, static_cast<bool (&)(scnr::Byte)>(looks_ascii));
}

bool looks_iso8859_1(scnr::StreamData stream) {
  return looks(stream, static_cast<bool (&)(scnr::Byte)>(looks_iso8859_1));
}

bool looks_extended_ascii(scnr::StreamData stream) {
  return looks(stream, static_cast<bool (&)(scnr::Byte)>(looks_extended_ascii));
}

bool valid_ucodepoint(int64_t code) {
  // check https://stackoverflow.com/questions/27415935/does-unicode-have-a-defined-maximum-number-of-code-points

  // Unicode v15 contains 149186 characters and this number is growing.
  // Let's check this code in some reasonable limit.
  return code < 150000;
}

bool looks_utf8(scnr::StreamData stream) {
  scnr::Byte buf[1024];
  // chunksize must be smaller than buffer size so that extra bytes can be read if needed
  constexpr size_t chunksize = 1000;
  while (auto nbytes = stream.readnext(buf, chunksize)) {
    for (size_t i = 0; i < nbytes; ++i) {
      int blocks = std::countl_one(buf[i]);
      if (blocks == 0) {
        // 0xxxxxxx is plain ASCII
        // looks_ascii(buf[i])?
        continue;
      }
      if (blocks == 1 || blocks > 4) {
        // invalid UTF-8
        return false;
      }
      blocks -= 1;

      // read some extra
      if (i + blocks >= nbytes) {
        // is ok?
        if (!stream.readnext(buf + chunksize, i + blocks - nbytes + 1)) {
          // unexpected eof
          return false;
        }
      }
      // int64_t code = 0;
      while (blocks) {
        i += 1;
        if (std::countl_one(buf[i]) != 1) {
          return false;
        }
        blocks -= 1;
      }
    }
  }
  return true;
}

bool looks_utf8_with_BOM(scnr::StreamData stream) {
  scnr::Byte buf[3];
  if (stream.read(buf, 0, 3) && buf[0] == 0xef && buf[1] == 0xbb && buf[2] == 0xbf) {
    return looks_utf8(stream.advanced(3));
  }
  return 0;
}

bool looks_utf32(scnr::StreamData stream, bool bigendian) {
  scnr::Byte buf[1024];
  while (auto nbytes = stream.readnext(buf, sizeof(buf))) {
    if (nbytes % 4 != 0) {
      return {};
    }
    for (int i = 0; i < nbytes; i += 4) {
      uint64_t code = 0;
      if (bigendian) {
        code = buf[i + 0] << 0x18 | buf[i + 1] << 0x10 | buf[i + 2] << 0x8 | buf[i + 3] << 0x0;
      } else {
        code = buf[i + 0] << 0x0 | buf[i + 1] << 0x8 | buf[i + 2] << 0x10 | buf[i + 3] << 0x18;
      }
      if (!valid_ucodepoint(code)) {
        return false;
      }
    }
  }
  return true;
}

bool looks_utf32_with_BOM(scnr::StreamData stream, bool bigendian) {
  scnr::Byte buf[4];
  if (!stream.read(buf, 0, 4)) {
    return false;
  }
  if (bigendian && buf[0] == 0 && buf[1] == 0 && buf[2] == 0xfe && buf[3] == 0xff &&
      looks_utf32(stream.advanced(4), bigendian)) {
    return true;
  }
  if (not bigendian && buf[0] == 0xff && buf[1] == 0xfe && buf[2] == 0 && buf[3] == 0 &&
      looks_utf32(stream.advanced(4), bigendian)) {
    return true;
  }
  return false;
}

#define F 0 /* character never appears in text */
#define T 1 /* character appears in plain ASCII text */
#define I 2 /* character appears in ISO-8859 text */
#define X 3 /* character appears in non-ISO extended ASCII (Mac, IBM PC) */

}  // namespace

namespace scnr {

std::optional<TxtFile> try_txt(scnr::StreamData stream) {
  TxtFile txtfile;
  if (looks_ascii(stream)) {
    txtfile.encoding = "ASCII";
  } else if (looks_utf8_with_BOM(stream)) {
    txtfile.encoding = "UTF-8";
    txtfile.withbom = true;
  } else if (looks_utf8(stream)) {
    txtfile.encoding = "UTF-8";
  } else if (looks_utf32_with_BOM(stream, true)) {
    txtfile.encoding = "UTF-32-BE";
    txtfile.withbom = true;
  } else if (looks_utf32_with_BOM(stream, false)) {
    txtfile.encoding = "UTF-32-LE";
    txtfile.withbom = true;
  } else if (looks_utf32(stream, true)) {
    txtfile.encoding = "UTF-32-BE";
  } else if (looks_utf32(stream, false)) {
    txtfile.encoding = "UTF-32-LE";
  } else if (looks_iso8859_1(stream)) {
    txtfile.encoding = "iso-8859-1";
  } else if (looks_extended_ascii(stream)) {
    txtfile.encoding = "extended ascii";
  } else {
    return {};
  }
  return txtfile;
}

}  // namespace scnr