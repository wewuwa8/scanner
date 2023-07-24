#include <scnr/parse_pe.hpp>

#include <bit>
#include <optional>

#include <scnr/pe/pe.h>

namespace scnr {

std::string_view CpuTypeAsSV(WORD cputype) {
  std::string_view retval;
#define SCNR_CASE_MACRO(code)                                                                                          \
  case code:                                                                                                           \
    retval = #code;                                                                                                    \
    break

  switch (cputype) {
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_UNKNOWN);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_AM33);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_AMD64);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_ARM);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_ARMNT);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_ARM64);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_EBC);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_I386);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_IA64);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_M32R);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_MIPS16);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_MIPSFPU);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_MIPSFPU16);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_POWERPC);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_POWERPCFP);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_R4000);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_SH3);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_SH3DSP);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_SH4);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_SH5);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_THUMB);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_WCEMIPSV2);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_TARGET_HOST);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_R3000);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_R10000);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_ALPHA);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_SH3E);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_ALPHA64);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_TRICORE);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_CEF);
    SCNR_CASE_MACRO(IMAGE_FILE_MACHINE_CEE);
    default:
      return {};
  }
#undef SCNR_MACRO_CASE

  retval.remove_prefix(std::strlen("IMAGE_FILE_MACHINE_"));
  return retval;
}

std::optional<scnr::PEFile> try_pe(scnr::StreamData stream) {
  scnr::PEFile pe_file;
  pe_file.endian = std::endian::little;

  const bool diff_endian = std::endian::native != std::endian::little;
  size_t read_bytes = 0;

  IMAGE_DOS_HEADER dos_header;
  if (!stream.readAs(0, dos_header)) {
    return {};
  }
  if (scnr::rev_bytes(dos_header.e_magic, diff_endian) != 0x5a4d) {
    return {};
  }
  read_bytes += sizeof(dos_header);
  const int64_t lfanew = scnr::rev_bytes(dos_header.e_lfanew, diff_endian);

  if (lfanew < 0) {
    return {};
  }

  IMAGE_NT_HEADERS32 nt_headers32;
  IMAGE_NT_HEADERS64 nt_headers64;
  if (!stream.readAs(lfanew, nt_headers32)) {
    return {};
  }
  if (scnr::rev_bytes(nt_headers32.Signature, diff_endian) != IMAGE_NT_SIGNATURE) {
    return {};
  }
  if (scnr::rev_bytes(nt_headers32.OptionalHeader.Magic, diff_endian) == IMAGE_NT_OPTIONAL_HDR64_MAGIC) {
    pe_file.w64 = true;
    if (!stream.readAs(lfanew, nt_headers64)) {
      return {};
    }
  } else {
    pe_file.w64 = false;
  }
  const auto machine =
    scnr::rev_bytes(pe_file.w64 ? nt_headers64.FileHeader.Machine : nt_headers32.FileHeader.Machine, diff_endian);
  pe_file.cputype = CpuTypeAsSV(machine);

  if (pe_file.w64) {
    pe_file.managed =
      nt_headers64.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress != 0;
  } else {
    pe_file.managed =
      nt_headers32.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR].VirtualAddress != 0;
  }

  return pe_file;
}

}  // namespace scnr
