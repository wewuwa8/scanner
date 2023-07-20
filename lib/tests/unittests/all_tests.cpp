#include <scnr/scnr.hpp>
#include <scnr/util.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <vector>

#include <gtest/gtest.h>

TEST(Encoding, Ascii) {
  auto file = scnr::read_file(std::filesystem::path{"ascii.txt"});
  auto fileinfo = scnr::detect_content(file);
  EXPECT_TRUE(std::holds_alternative<scnr::TxtFile>(fileinfo));
  EXPECT_EQ(std::get<scnr::TxtFile>(fileinfo).encoding, "ASCII");
  scnr::TxtFile txt;
  txt.encoding = "ASCII";

  scnr::FileInfo fi = txt;
  EXPECT_EQ(fi, fileinfo);
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
    auto file = scnr::read_file(dir_entry.path());
    auto fileinfo = scnr::detect_content(file);
    std::cout << "path = " << dir_entry.path() << ", " << fileinfo << '\n';
  }
}

// Define a test case
class TParam : public ::testing::TestWithParam<std::pair<std::string, scnr::FileInfo>> {};

TEST_P(TParam, DetectContent) {
  auto [pathstr, expected] = GetParam();
  auto file = scnr::read_file(std::filesystem::path{pathstr});
  auto fileinfo = scnr::detect_content(file);
  EXPECT_EQ(fileinfo, expected);
}

// clang-format off
INSTANTIATE_TEST_SUITE_P(UnitestsData, TParam, ::testing::Values(
    std::make_pair("ascii.txt", scnr::TxtFile{.encoding = "ASCII"}),
    std::make_pair("utf8.txt", scnr::TxtFile{.encoding = "UTF-8"}),
    std::make_pair("utf8-bom.txt", scnr::TxtFile{.encoding = "UTF-8", .withbom = true}),
    // std::make_pair("utf16-be.txt", scnr::TxtFile{.encoding = "UTF-16"}),
    std::make_pair("utf32-le.txt", scnr::TxtFile{.encoding = "UTF-32-LE"}),
    std::make_pair("utf32-be-bom.txt", scnr::TxtFile{.encoding = "UTF-32-BE", .withbom = true}),
    std::make_pair("iso-8859-1.txt", scnr::TxtFile{.encoding = "iso-8859-1"})
    // std::make_pair("win1252.txt", scnr::TxtFile{.encoding = "WIN1252"})
), [](const auto& info) {
    auto testname = info.param.first;
    // gtest forbids having 'complex' chars in the testname
    std::replace_if(testname.begin(), testname.end(), [](char c) {
      return !std::isalnum(c);
    }, '_');

    return testname;
  });
// clang-format on
