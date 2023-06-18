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

std::optional<scnr::MachOSingle> parse_single(const scnr::Byte* buf, size_t nbytes) {
  if (nbytes < sizeof(uint32_t)) {
    return {};
  }

  const uint32_t magic = scnr::ReadAs<uint32_t>(buf);
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
      if (nbytes < sizeof(mach_header)) {
        return {};
      }
      w64 = false;
      header32 = scnr::ReadAs<mach_header>(buf);

      break;
    case MH_CIGAM_64:
      diff_endian = true;
      [[fallthrough]];

    case MH_MAGIC_64:
      if (nbytes < sizeof(mach_header_64)) {
        return {};
      }
      w64 = true;
      header64 = scnr::ReadAs<mach_header_64>(buf);
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
    if (nbytes < read_bytes + sizeof(load_command)) {
      return {};
    }

    load_command lc = scnr::ReadAs<load_command>(buf + read_bytes);
    auto lc_cmd = diff_endian ? scnr::rev_bytes(lc.cmd) : lc.cmd;
    auto lc_cmdsize = diff_endian ? scnr::rev_bytes(lc.cmdsize) : lc.cmdsize;

    if (lc_cmd == LC_CODE_SIGNATURE) {
      if (nbytes < read_bytes + sizeof(linkedit_data_command)) {
        return {};
      }

      linkedit_data_command le_data_cmd_sign = scnr::ReadAs<linkedit_data_command>(buf + read_bytes);
      auto dataoff = diff_endian ? scnr::rev_bytes(le_data_cmd_sign.dataoff) : le_data_cmd_sign.dataoff;
      auto datasize = diff_endian ? scnr::rev_bytes(le_data_cmd_sign.datasize) : le_data_cmd_sign.datasize;
      retval.issigned = true;
    }

    read_bytes += lc_cmdsize;
  }

  return retval;
}

std::optional<scnr::MachOFat> parse_fat(const scnr::Byte* buf, size_t nbytes) {
  if (nbytes < sizeof(fat_header)) {
    return {};
  }
  size_t read_bytes = 0;
  const fat_header header = scnr::ReadAs<fat_header>(buf);
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
    if (nbytes < read_bytes + sizeof(fat_arch)) {
      return {};
    }
    auto arch = scnr::ReadAs<fat_arch>(buf + read_bytes);
    auto offset = diff_endian ? scnr::rev_bytes(arch.offset) : arch.offset;
    auto size = diff_endian ? scnr::rev_bytes(arch.size) : arch.size;
    auto macho = parse_single(buf + offset, size);
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

std::optional<MachOFile> try_macho(const Byte* buf, size_t nbytes) {
  auto fat = parse_fat(buf, nbytes);
  if (fat) {
    return MachOFile{fat.value()};
  }
  auto single = parse_single(buf, nbytes);
  if (single) {
    return MachOFile{single.value()};
  }
  return {};
}

}  // namespace scnr
