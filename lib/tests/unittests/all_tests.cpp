#include <scnr/scnr.hpp>
#include <scnr/util.hpp>

#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#include <gtest/gtest.h>

TEST(Encoding, Ascii) {
  auto buffer = scnr::read_file(std::filesystem::path{"ascii.txt"});
  auto fileinfo = scnr::detect_content(buffer.data(), buffer.size());

  for (char c : buffer) {
    std::cout << c;
  }
  std::cout << '\n';
  EXPECT_EQ(0, 1);
}

TEST(Encoding, All) {
  std::filesystem::directory_iterator dir_iter(".");
  for (const auto& dir_entry : dir_iter) {
    auto buffer = scnr::read_file(dir_entry.path());
    auto fileinfo = scnr::detect_content(buffer.data(), buffer.size());
    std::cout << "path = " << dir_entry.path() << ", " << fileinfo << '\n';
  }

  EXPECT_EQ(0, 1);
}
