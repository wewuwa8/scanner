#pragma once

#include <scnr/types.hpp>

#include <algorithm>
#include <bit>
#include <cctype>
#include <cstring>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

namespace scnr {

template <typename T>
void MyLogImpl(const T& value) {
  std::cout << value << "\n";
}

template <typename T, typename... TA>
void MyLogImpl(const T& head, const TA&... tail) {
  std::cout << head << ", ";
  MyLogImpl(tail...);
}

#define SCNR_STRINGIFY_IMPL(x) #x
#define SCNR_STRINGIFY(x) SCNR_STRINGIFY_IMPL(x)

#define MyLog(...) scnr::MyLogImpl(__FILE__ ":" SCNR_STRINGIFY(__LINE__), __VA_ARGS__)

#define MyAssertImpl(WHAT, COND)                                                                                       \
  do {                                                                                                                 \
    if (!(COND)) {                                                                                                     \
      MyLog("'" WHAT "' failed!");                                                                                     \
      std::terminate();                                                                                                \
    }                                                                                                                  \
  } while (false)

#define MyAssert(...) MyAssertImpl(#__VA_ARGS__, __VA_ARGS__)

inline std::vector<Byte> read_file(const std::filesystem::path& path) {
  std::fstream t(path, std::ios::in | std::ios::binary);
  t.seekg(0, std::ios::end);
  size_t size = t.tellg();
  std::vector<Byte> buffer(size);
  t.seekg(0);
  t.read(reinterpret_cast<char*>(buffer.data()), size);
  return buffer;
}

template <typename T>
T rev_bytes(T x, bool doit = true) {
  static_assert(std::is_integral<T>::value, "Integral required.");

  if (not doit) {
    return x;
  }

  T y;
  auto ybuf = reinterpret_cast<unsigned char*>(&y);
  std::memcpy(ybuf, &x, sizeof(T));
  std::reverse(ybuf, ybuf + sizeof(T));
  return y;
}

template <typename T>
T BEToHost(T x) {
  if constexpr (std::endian::native == std::endian::big) {
    return x;
  } else {
    return rev_bytes(x);
  }
}

template <typename T>
T LEToHost(T x) {
  if constexpr (std::endian::native == std::endian::little) {
    return x;
  } else {
    return rev_bytes(x);
  }
}

template <typename T>
T AnyToHost(T x, bool bigendian = true) {
  if (bigendian) {
    return BEToHost(x);
  }
  return LEToHost(x);
}

template <typename T>
T ReadAs(const Byte* buf) {
  T retval;
  std::memcpy(&retval, buf, sizeof(T));
  return retval;
}

inline std::endian GetEndian(bool opposite = false) {
  if (not opposite) {
    return std::endian::native;
  }
  if (std::endian::native == std::endian::big) {
    return std::endian::little;
  }
  return std::endian::big;
}

inline void trim_right(std::string& str) {
  while (!str.empty() && (std::isspace(str.back()) || str.back() == '\0')) {
    str.pop_back();
  }
}

// poor man's hash_combine
inline void hash_combine(std::uint64_t& seed, std::uint64_t hash) {
  seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

}  // namespace scnr