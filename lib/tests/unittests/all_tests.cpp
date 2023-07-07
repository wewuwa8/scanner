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
  EXPECT_TRUE(std::holds_alternative<scnr::TxtFile>(fileinfo));
  EXPECT_EQ(std::get<scnr::TxtFile>(fileinfo).encoding, "ASCII");
}

TEST(Encoding, All) {
  std::filesystem::directory_iterator dir_iter(".");
  for (const auto& dir_entry : dir_iter) {
    if (not dir_entry.is_regular_file()) {
      std::cout << "skipping path = " << dir_entry.path() << std::endl;
      continue;
    } else {
      std::cout << "trying path = " << dir_entry.path() << std::endl;
    }
    auto buffer = scnr::read_file(dir_entry.path());
    auto fileinfo = scnr::detect_content(buffer.data(), buffer.size());
    std::cout << "path = " << dir_entry.path() << ", " << fileinfo << '\n';
  }
}
