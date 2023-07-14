#include <scnr/file.hpp>
#include <scnr/parse_mach-o.hpp>
#include <scnr/util.hpp>

#include <cstring>
#include <string_view>

#include <scnr/mach-o/fat.h>
#include <scnr/mach-o/loader.h>

namespace {
std::string_view CpuTypeAsSV(cpu_type_t cputype) {
  std::string_view retval;
#define SCNR_CASE_MACRO(code)                                                                                          \
  case code:                                                                                                           \
    retval = #code;                                                                                                    \
    break

  switch (cputype) {
    SCNR_CASE_MACRO(CPU_TYPE_ANY);
    SCNR_CASE_MACRO(CPU_TYPE_VAX);
    SCNR_CASE_MACRO(CPU_TYPE_MC680x0);
    SCNR_CASE_MACRO(CPU_TYPE_X86);
    SCNR_CASE_MACRO(CPU_TYPE_X86_64);
    SCNR_CASE_MACRO(CPU_TYPE_MC98000);
    SCNR_CASE_MACRO(CPU_TYPE_HPPA);
    SCNR_CASE_MACRO(CPU_TYPE_ARM);
    SCNR_CASE_MACRO(CPU_TYPE_ARM64);
    SCNR_CASE_MACRO(CPU_TYPE_ARM64_32);
    SCNR_CASE_MACRO(CPU_TYPE_MC88000);
    SCNR_CASE_MACRO(CPU_TYPE_SPARC);
    SCNR_CASE_MACRO(CPU_TYPE_I860);
    SCNR_CASE_MACRO(CPU_TYPE_POWERPC);
    SCNR_CASE_MACRO(CPU_TYPE_POWERPC64);
    default:
      return {};
  }
#undef SCNR_MACRO_CASE

  retval.remove_prefix(std::strlen("CPU_TYPE_"));
  return retval;
}

std::optional<scnr::MachOSingle> parse_single(scnr::StreamData stream) {
  uint32_t magic;
  if (!stream.readAs(0, magic)) {
    return {};
  }
  mach_header header32;
  mach_header_64 header64;

  size_t read_bytes = 0;
  scnr::MachOSingle retval;
  bool diff_endian = false;
  bool fat = false;
  bool w64 = true;

  switch (magic) {
    case MH_CIGAM:
      diff_endian = true;
      [[fallthrough]];
    case MH_MAGIC:
      if (!stream.readAs(read_bytes, header32)) {
        return {};
      }
      w64 = false;

      break;
    case MH_CIGAM_64:
      diff_endian = true;
      [[fallthrough]];

    case MH_MAGIC_64:
      if (!stream.readAs(read_bytes, header64)) {
        return {};
      }
      w64 = true;
      break;

    default:
      return {};
  }
  read_bytes += w64 ? sizeof(mach_header_64) : sizeof(mach_header);

  auto cputype = w64 ? header64.cputype : header32.cputype;
  if (diff_endian) {
    cputype = scnr::rev_bytes(cputype);
  }

  retval.w64 = w64;
  retval.endian = scnr::GetEndian(diff_endian);
  retval.cputype = CpuTypeAsSV(cputype);

  auto ncmds = w64 ? header64.ncmds : header32.ncmds;
  if (diff_endian) {
    ncmds = scnr::rev_bytes(ncmds);
  }

  for (uint32_t i = 0; i < ncmds; ++i) {
    load_command lc;
    if (!stream.readAs(read_bytes, lc)) {
      return {};
    }
    auto lc_cmd = diff_endian ? scnr::rev_bytes(lc.cmd) : lc.cmd;
    auto lc_cmdsize = diff_endian ? scnr::rev_bytes(lc.cmdsize) : lc.cmdsize;

    if (lc_cmd == LC_CODE_SIGNATURE) {
      linkedit_data_command le_data_cmd_sign;
      if (!stream.readAs(read_bytes, le_data_cmd_sign)) {
        return {};
      }
      auto dataoff = diff_endian ? scnr::rev_bytes(le_data_cmd_sign.dataoff) : le_data_cmd_sign.dataoff;
      auto datasize = diff_endian ? scnr::rev_bytes(le_data_cmd_sign.datasize) : le_data_cmd_sign.datasize;
      retval.issigned = true;
    }

    read_bytes += lc_cmdsize;
  }

  return retval;
}

std::optional<scnr::MachOFat> parse_fat(scnr::StreamData stream) {
  size_t read_bytes = 0;
  fat_header header;
  if (!stream.readAs(read_bytes, header)) {
    return {};
  }
  bool diff_endian = false;

  switch (header.magic) {
    case FAT_CIGAM:
      diff_endian = true;
      [[fallthrough]];
    case FAT_MAGIC:
      break;
    default:
      return {};
  }

  read_bytes += sizeof(fat_header);
  scnr::MachOFat retval;

  uint32_t nfat_arch = diff_endian ? scnr::rev_bytes(header.nfat_arch) : header.nfat_arch;
  if (nfat_arch == 0) {
    return {};
  }
  for (int i = 0; i < nfat_arch; ++i) {
    fat_arch arch;
    if (!stream.readAs(read_bytes, arch)) {
      return {};
    }
    auto offset = diff_endian ? scnr::rev_bytes(arch.offset) : arch.offset;
    auto size = diff_endian ? scnr::rev_bytes(arch.size) : arch.size;
    auto macho = parse_single(stream.advanced(offset));
    if (!macho) {
      return {};
    }
    retval.files.push_back(std::move(macho.value()));

    read_bytes += sizeof(fat_arch);
  }
  return retval;
}

};  // namespace

namespace scnr {

std::optional<MachOFile> try_macho(scnr::StreamData stream) {
  auto fat = parse_fat(stream);
  if (fat) {
    return MachOFile{fat.value()};
  }
  auto single = parse_single(stream);
  if (single) {
    return MachOFile{single.value()};
  }
  return {};
}

}  // namespace scnr
