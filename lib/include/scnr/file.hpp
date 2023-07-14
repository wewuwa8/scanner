#pragma once

#include <scnr/types.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>

namespace scnr {
class StreamData {
 public:
  StreamData(std::istream* stream = nullptr, size_t offset = 0) : stream_(stream), offset_(offset) {
  }

  bool read(char* dst, size_t from, size_t count) const {
    return read(reinterpret_cast<Byte*>(dst), from, count);
  }

  size_t readnext(Byte* dst, size_t count) const {
    return readsome(dst, nextpos_, count);
  }

  size_t readsome(Byte* dst, size_t from, size_t count) const {
    if (not stream_) {
      return 0;
    }
    stream_->clear();
    stream_->seekg(offset_ + from);
    stream_->read(reinterpret_cast<char*>(dst), count);
    auto gcount = stream_->gcount();
    nextpos_ = from + gcount;
    return gcount;
  }

  bool read(Byte* dst, size_t from, size_t count) const {
    return readsome(dst, from, count) == count;
  };

  template <typename T>
  bool readAs(size_t from, T& result) const {
    return read(reinterpret_cast<Byte*>(std::addressof(result)), from, sizeof(result));
  }

  StreamData advanced(size_t offset) const {
    return StreamData(stream_, offset_ + offset);
  }

 private:
  mutable std::istream* stream_;
  const size_t offset_ = 0;
  mutable size_t nextpos_ = 0;
};

class File {
 public:
  File(std::filesystem::path path)
    : path_(std::move(path)), fstream_(std::make_unique<std::fstream>(path_, std::ios::in | std::ios::binary)) {
  }

  operator StreamData() const {
    return StreamData(fstream_.get());
  }

 private:
  std::filesystem::path path_;
  std::unique_ptr<std::fstream> fstream_;
};

}  // namespace scnr