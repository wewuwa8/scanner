#include <scnr/scnr.hpp>
#include <scnr/util.hpp>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

TEST(DetectContent, Ascii) {
  auto file = scnr::read_file(std::filesystem::path{"ascii.txt"});
  auto fileinfo = scnr::detect_content(file);
  EXPECT_EQ(scnr::FileInfo{scnr::TxtFile{.encoding = "ASCII"}}, fileinfo);
}

TEST(DetectContent, Nonexist) {
  try {
    auto file = scnr::read_file(std::filesystem::path{"nonexist"});
  } catch (const std::runtime_error& ex) {
    EXPECT_STREQ(ex.what(), R"(Could not open file 'nonexist': is not a regular file)");
    return;
  }
  FAIL() << "Expected exception!";
}

TEST(DetectContent, All) {
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

class TParam : public ::testing::TestWithParam<std::pair<std::string, scnr::FileInfo>> {};

TEST_P(TParam, DetectContent) {
  auto [pathstr, expected] = GetParam();
  auto file = scnr::read_file(std::filesystem::path{pathstr});
  auto fileinfo = scnr::detect_content(file);
  EXPECT_EQ(fileinfo, expected);
}

INSTANTIATE_TEST_SUITE_P(
  UnitestsData, TParam,
  ::testing::Values(
    std::make_pair("ascii.txt", scnr::TxtFile{.encoding = "ASCII"}),
    std::make_pair("utf8.txt", scnr::TxtFile{.encoding = "UTF-8"}),
    std::make_pair("utf8-bom.txt", scnr::TxtFile{.encoding = "UTF-8", .withbom = true}),
    std::make_pair("utf32-le.txt", scnr::TxtFile{.encoding = "UTF-32-LE"}),
    std::make_pair("utf32-be-bom.txt", scnr::TxtFile{.encoding = "UTF-32-BE", .withbom = true}),
    std::make_pair("xml-iso-8859-1.xml", scnr::XmlFile{.encoding = "iso-8859-1"}),
    std::make_pair("xml-ascii.xml", scnr::XmlFile{.encoding = "ASCII"}),

    std::make_pair("amd64.exe", scnr::PEFile{.endian = std::endian::little, .w64 = true, .cputype = "AMD64"}),
    std::make_pair("elf-64-x86.elf", scnr::ElfFile{.endian = std::endian::little,
                                                   .w64 = true,
                                                   .cputype = "X86_64",
                                                   .interpreter = "/lib64/ld-linux-x86-64.so.2"}),
    std::make_pair("mach-o-fat.o",
                   scnr::MachOFile{.value = scnr::MachOFat{.files = {scnr::MachOSingle{.endian = std::endian::little,
                                                                                       .w64 = true,
                                                                                       .cputype = "X86_64"},
                                                                     scnr::MachOSingle{.endian = std::endian::little,
                                                                                       .w64 = true,
                                                                                       .cputype = "ARM64",
                                                                                       .issigned = true}}}})),
  [](const auto& info) {
    auto testname = info.param.first;
    // gtest forbids having 'complex' chars in the testname
    std::replace_if(
      testname.begin(), testname.end(),
      [](char c) {
        return !std::isalnum(c);
      },
      '_');

    return testname;
  });
