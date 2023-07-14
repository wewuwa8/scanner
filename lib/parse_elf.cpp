#include <scnr/file.hpp>
#include <scnr/parse_elf.hpp>
#include <scnr/types.hpp>
#include <scnr/util.hpp>

#include <optional>
#include <type_traits>

#include <scnr/elf/elf.h>

namespace {
std::string_view CpuTypeAsSV(Elf64_Half cputype) {
  std::string_view retval;

#define SCNR_CASE_MACRO(code)                                                                                          \
  case code:                                                                                                           \
    retval = #code;                                                                                                    \
    break
  switch (cputype) {
    SCNR_CASE_MACRO(EM_NONE);
    SCNR_CASE_MACRO(EM_M32);
    SCNR_CASE_MACRO(EM_SPARC);
    SCNR_CASE_MACRO(EM_386);
    SCNR_CASE_MACRO(EM_68K);
    SCNR_CASE_MACRO(EM_88K);
    SCNR_CASE_MACRO(EM_IAMCU);
    SCNR_CASE_MACRO(EM_860);
    SCNR_CASE_MACRO(EM_MIPS);
    SCNR_CASE_MACRO(EM_S370);
    SCNR_CASE_MACRO(EM_MIPS_RS3_LE);
    SCNR_CASE_MACRO(EM_PARISC);
    SCNR_CASE_MACRO(EM_VPP500);
    SCNR_CASE_MACRO(EM_SPARC32PLUS);
    SCNR_CASE_MACRO(EM_960);
    SCNR_CASE_MACRO(EM_PPC);
    SCNR_CASE_MACRO(EM_PPC64);
    SCNR_CASE_MACRO(EM_S390);
    SCNR_CASE_MACRO(EM_SPU);
    SCNR_CASE_MACRO(EM_V800);
    SCNR_CASE_MACRO(EM_FR20);
    SCNR_CASE_MACRO(EM_RH32);
    SCNR_CASE_MACRO(EM_RCE);
    SCNR_CASE_MACRO(EM_ARM);
    SCNR_CASE_MACRO(EM_FAKE_ALPHA);
    SCNR_CASE_MACRO(EM_SH);
    SCNR_CASE_MACRO(EM_SPARCV9);
    SCNR_CASE_MACRO(EM_TRICORE);
    SCNR_CASE_MACRO(EM_ARC);
    SCNR_CASE_MACRO(EM_H8_300);
    SCNR_CASE_MACRO(EM_H8_300H);
    SCNR_CASE_MACRO(EM_H8S);
    SCNR_CASE_MACRO(EM_H8_500);
    SCNR_CASE_MACRO(EM_IA_64);
    SCNR_CASE_MACRO(EM_MIPS_X);
    SCNR_CASE_MACRO(EM_COLDFIRE);
    SCNR_CASE_MACRO(EM_68HC12);
    SCNR_CASE_MACRO(EM_MMA);
    SCNR_CASE_MACRO(EM_PCP);
    SCNR_CASE_MACRO(EM_NCPU);
    SCNR_CASE_MACRO(EM_NDR1);
    SCNR_CASE_MACRO(EM_STARCORE);
    SCNR_CASE_MACRO(EM_ME16);
    SCNR_CASE_MACRO(EM_ST100);
    SCNR_CASE_MACRO(EM_TINYJ);
    SCNR_CASE_MACRO(EM_X86_64);
    SCNR_CASE_MACRO(EM_PDSP);
    SCNR_CASE_MACRO(EM_PDP10);
    SCNR_CASE_MACRO(EM_PDP11);
    SCNR_CASE_MACRO(EM_FX66);
    SCNR_CASE_MACRO(EM_ST9PLUS);
    SCNR_CASE_MACRO(EM_ST7);
    SCNR_CASE_MACRO(EM_68HC16);
    SCNR_CASE_MACRO(EM_68HC11);
    SCNR_CASE_MACRO(EM_68HC08);
    SCNR_CASE_MACRO(EM_68HC05);
    SCNR_CASE_MACRO(EM_SVX);
    SCNR_CASE_MACRO(EM_ST19);
    SCNR_CASE_MACRO(EM_VAX);
    SCNR_CASE_MACRO(EM_CRIS);
    SCNR_CASE_MACRO(EM_JAVELIN);
    SCNR_CASE_MACRO(EM_FIREPATH);
    SCNR_CASE_MACRO(EM_ZSP);
    SCNR_CASE_MACRO(EM_MMIX);
    SCNR_CASE_MACRO(EM_HUANY);
    SCNR_CASE_MACRO(EM_PRISM);
    SCNR_CASE_MACRO(EM_AVR);
    SCNR_CASE_MACRO(EM_FR30);
    SCNR_CASE_MACRO(EM_D10V);
    SCNR_CASE_MACRO(EM_D30V);
    SCNR_CASE_MACRO(EM_V850);
    SCNR_CASE_MACRO(EM_M32R);
    SCNR_CASE_MACRO(EM_MN10300);
    SCNR_CASE_MACRO(EM_MN10200);
    SCNR_CASE_MACRO(EM_PJ);
    SCNR_CASE_MACRO(EM_OPENRISC);
    SCNR_CASE_MACRO(EM_ARC_COMPACT);
    SCNR_CASE_MACRO(EM_XTENSA);
    SCNR_CASE_MACRO(EM_VIDEOCORE);
    SCNR_CASE_MACRO(EM_TMM_GPP);
    SCNR_CASE_MACRO(EM_NS32K);
    SCNR_CASE_MACRO(EM_TPC);
    SCNR_CASE_MACRO(EM_SNP1K);
    SCNR_CASE_MACRO(EM_ST200);
    SCNR_CASE_MACRO(EM_IP2K);
    SCNR_CASE_MACRO(EM_MAX);
    SCNR_CASE_MACRO(EM_CR);
    SCNR_CASE_MACRO(EM_F2MC16);
    SCNR_CASE_MACRO(EM_MSP430);
    SCNR_CASE_MACRO(EM_BLACKFIN);
    SCNR_CASE_MACRO(EM_SE_C33);
    SCNR_CASE_MACRO(EM_SEP);
    SCNR_CASE_MACRO(EM_ARCA);
    SCNR_CASE_MACRO(EM_UNICORE);
    SCNR_CASE_MACRO(EM_EXCESS);
    SCNR_CASE_MACRO(EM_DXP);
    SCNR_CASE_MACRO(EM_ALTERA_NIOS2);
    SCNR_CASE_MACRO(EM_CRX);
    SCNR_CASE_MACRO(EM_XGATE);
    SCNR_CASE_MACRO(EM_C166);
    SCNR_CASE_MACRO(EM_M16C);
    SCNR_CASE_MACRO(EM_DSPIC30F);
    SCNR_CASE_MACRO(EM_CE);
    SCNR_CASE_MACRO(EM_M32C);
    SCNR_CASE_MACRO(EM_TSK3000);
    SCNR_CASE_MACRO(EM_RS08);
    SCNR_CASE_MACRO(EM_SHARC);
    SCNR_CASE_MACRO(EM_ECOG2);
    SCNR_CASE_MACRO(EM_SCORE7);
    SCNR_CASE_MACRO(EM_DSP24);
    SCNR_CASE_MACRO(EM_VIDEOCORE3);
    SCNR_CASE_MACRO(EM_LATTICEMICO32);
    SCNR_CASE_MACRO(EM_SE_C17);
    SCNR_CASE_MACRO(EM_TI_C6000);
    SCNR_CASE_MACRO(EM_TI_C2000);
    SCNR_CASE_MACRO(EM_TI_C5500);
    SCNR_CASE_MACRO(EM_TI_ARP32);
    SCNR_CASE_MACRO(EM_TI_PRU);
    SCNR_CASE_MACRO(EM_MMDSP_PLUS);
    SCNR_CASE_MACRO(EM_CYPRESS_M8C);
    SCNR_CASE_MACRO(EM_R32C);
    SCNR_CASE_MACRO(EM_TRIMEDIA);
    SCNR_CASE_MACRO(EM_QDSP6);
    SCNR_CASE_MACRO(EM_8051);
    SCNR_CASE_MACRO(EM_STXP7X);
    SCNR_CASE_MACRO(EM_NDS32);
    SCNR_CASE_MACRO(EM_ECOG1X);
    SCNR_CASE_MACRO(EM_MAXQ30);
    SCNR_CASE_MACRO(EM_XIMO16);
    SCNR_CASE_MACRO(EM_MANIK);
    SCNR_CASE_MACRO(EM_CRAYNV2);
    SCNR_CASE_MACRO(EM_RX);
    SCNR_CASE_MACRO(EM_METAG);
    SCNR_CASE_MACRO(EM_MCST_ELBRUS);
    SCNR_CASE_MACRO(EM_ECOG16);
    SCNR_CASE_MACRO(EM_CR16);
    SCNR_CASE_MACRO(EM_ETPU);
    SCNR_CASE_MACRO(EM_SLE9X);
    SCNR_CASE_MACRO(EM_L10M);
    SCNR_CASE_MACRO(EM_K10M);
    SCNR_CASE_MACRO(EM_AARCH64);
    SCNR_CASE_MACRO(EM_AVR32);
    SCNR_CASE_MACRO(EM_STM8);
    SCNR_CASE_MACRO(EM_TILE64);
    SCNR_CASE_MACRO(EM_TILEPRO);
    SCNR_CASE_MACRO(EM_MICROBLAZE);
    SCNR_CASE_MACRO(EM_CUDA);
    SCNR_CASE_MACRO(EM_TILEGX);
    SCNR_CASE_MACRO(EM_CLOUDSHIELD);
    SCNR_CASE_MACRO(EM_COREA_1ST);
    SCNR_CASE_MACRO(EM_COREA_2ND);
    SCNR_CASE_MACRO(EM_ARCV2);
    SCNR_CASE_MACRO(EM_OPEN8);
    SCNR_CASE_MACRO(EM_RL78);
    SCNR_CASE_MACRO(EM_VIDEOCORE5);
    SCNR_CASE_MACRO(EM_78KOR);
    SCNR_CASE_MACRO(EM_56800EX);
    SCNR_CASE_MACRO(EM_BA1);
    SCNR_CASE_MACRO(EM_BA2);
    SCNR_CASE_MACRO(EM_XCORE);
    SCNR_CASE_MACRO(EM_MCHP_PIC);
    SCNR_CASE_MACRO(EM_INTELGT);
    SCNR_CASE_MACRO(EM_KM32);
    SCNR_CASE_MACRO(EM_KMX32);
    SCNR_CASE_MACRO(EM_EMX16);
    SCNR_CASE_MACRO(EM_EMX8);
    SCNR_CASE_MACRO(EM_KVARC);
    SCNR_CASE_MACRO(EM_CDP);
    SCNR_CASE_MACRO(EM_COGE);
    SCNR_CASE_MACRO(EM_COOL);
    SCNR_CASE_MACRO(EM_NORC);
    SCNR_CASE_MACRO(EM_CSR_KALIMBA);
    SCNR_CASE_MACRO(EM_Z80);
    SCNR_CASE_MACRO(EM_VISIUM);
    SCNR_CASE_MACRO(EM_FT32);
    SCNR_CASE_MACRO(EM_MOXIE);
    SCNR_CASE_MACRO(EM_AMDGPU);
    SCNR_CASE_MACRO(EM_RISCV);
    SCNR_CASE_MACRO(EM_BPF);
    SCNR_CASE_MACRO(EM_CSKY);
    SCNR_CASE_MACRO(EM_LOONGARCH);
    default:
      return {};
  }
#undef SCNR_MACRO_CASE

  retval.remove_prefix(3);  // EM_
  return retval;
}

struct Elf32Traits {
  static constexpr bool w64 = false;
  using Elf_Ehdr = Elf32_Ehdr;
  using Elf_Phdr = Elf32_Phdr;
};

struct Elf64Traits {
  static constexpr bool w64 = true;
  using Elf_Ehdr = Elf64_Ehdr;
  using Elf_Phdr = Elf64_Phdr;
};

template <typename ElfTraits>
std::optional<scnr::ElfFile> try_elf_impl(scnr::StreamData stream) {
  scnr::Byte buf[EI_NIDENT + 1];
  if (!stream.read(buf, 0, EI_NIDENT + 1) || buf[EI_MAG0] != ELFMAG0 || buf[EI_MAG1] != ELFMAG1 ||
      buf[EI_MAG2] != ELFMAG2 || buf[EI_MAG3] != ELFMAG3) {
    return {};
  }
  size_t read_bytes = 0;

  scnr::ElfFile elffile;
  elffile.w64 = ElfTraits::w64;
  typename ElfTraits::Elf_Ehdr elf_hdr;
  if (!stream.readAs(read_bytes, elf_hdr)) {
    return {};
  }

  switch (buf[EI_DATA]) {
    case ELFDATA2LSB:
      elffile.endian = std::endian::little;
      break;
    case ELFDATA2MSB:
      elffile.endian = std::endian::big;
    default:
      break;
  }
  bool diff_endian = elffile.endian != std::endian::native;

  auto cputype = scnr::rev_bytes(elf_hdr.e_machine, diff_endian);
  auto phoff = scnr::rev_bytes(elf_hdr.e_phoff, diff_endian);
  auto phnum = scnr::rev_bytes(elf_hdr.e_phnum, diff_endian);
  auto phentsize = scnr::rev_bytes(elf_hdr.e_phentsize, diff_endian);

  elffile.cputype = CpuTypeAsSV(cputype);

  typename ElfTraits::Elf_Phdr prg_hdr;

  for (int i = 0; i < phnum; ++i) {
    if (!stream.readAs(phoff + i * phentsize, prg_hdr)) {
      return {};
    }

    auto type = scnr::rev_bytes(prg_hdr.p_type, diff_endian);
    if (type == PT_INTERP) {
      auto sz = scnr::rev_bytes(prg_hdr.p_filesz, diff_endian);
      auto offset = scnr::rev_bytes(prg_hdr.p_offset, diff_endian);

      if (sz > 4098) {
        // sanity check
        return {};
      }

      std::string interpreter(sz, '\0');
      if (!stream.read(interpreter.data(), offset, sz)) {
        return {};
      }
      elffile.interpreter = std::move(interpreter);
      scnr::trim_right(elffile.interpreter);
    }
  }
  return elffile;
}

}  // namespace

namespace scnr {
std::optional<ElfFile> try_elf(scnr::StreamData stream) {
  Byte buf[EI_CLASS + 1];
  if (not stream.read(buf, 0, EI_CLASS + 1)) {
    return {};
  }
  if (buf[EI_CLASS] == ELFCLASS32) {
    return try_elf_impl<Elf32Traits>(stream);
  }
  if (buf[EI_CLASS] == ELFCLASS64) {
    return try_elf_impl<Elf64Traits>(stream);
  }
  return {};
}

}  // namespace scnr