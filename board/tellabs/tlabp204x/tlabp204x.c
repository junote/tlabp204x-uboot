/*
 * Copyright 2011,2012 Freescale Semiconductor, Inc.
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

#include <common.h>
#include <command.h>
#include <netdev.h>
#include <linux/compiler.h>
#include <asm/mmu.h>
#include <asm/processor.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_law.h>
#include <asm/fsl_serdes.h>
#include <asm/fsl_liodn.h>
#include <fm_eth.h>

#include "mvswitch.h"
extern void pci_of_setup(void *blob, bd_t *bd);

#include "cpld.h"

DECLARE_GLOBAL_DATA_PTR;

int checkboard(void)
{
	/* u8 sw; */
	struct cpu_type *cpu = gd->arch.cpu;
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	unsigned int i;

	printf("Board: %s, ", cpu->name);
	printf("CPLD 0x%02x ver %d. ", ((unsigned int) upcpld_read(0x00)),
	       upcpld_read(0x01));
  #ifdef CONFIG_PHYS_64BIT
	    puts("(36-bit Addressing)\n");
  #else
	    puts("\n");
  #endif

  /* printf("CPLD version: %d.%d ", CPLD_READ(cpld_ver), */
	/* 		CPLD_READ(cpld_ver_sub)); */

	/* sw = CPLD_READ(fbank_sel); */
	/* printf("vBank: %d\n", sw & 0x1); */

	/*
	 * Display the actual SERDES reference clocks as configured by the
	 * dip switches on the board.  Note that the SWx registers could
	 * technically be set to force the reference clocks to match the
	 * values that the SERDES expects (or vice versa).  For now, however,
	 * we just display both values and hope the user notices when they
	 * don't match.
	 */
	/* puts("SERDES Reference Clocks: "); */
	/* sw = in_8(&CPLD_SW(2)) >> 2; */
	/* for (i = 0; i < 2; i++) { */
	/* 	static const char * const freq[][3] = {{"0", "100", "125"}, */
	/* 					{"100", "156.25", "125"} */
	/* 	}; */
	/* 	unsigned int clock = (sw >> (2 * i)) & 3; */

	/* 	printf("Bank%u=%sMhz ", i+1, freq[i][clock]); */
	/* } */
	/* puts("\n"); */

	return 0;
}

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

	/* board only uses the DDR_MCK0/1, so disable the DDR_MCK2/3 */
	setbits_be32(&gur->ddrclkdr, 0x000f000f);

	return 0;
}

/* #define CPLD_LANE_A_SEL	0x1 */
/* #define CPLD_LANE_G_SEL	0x2 */
/* #define CPLD_LANE_C_SEL	0x4 */
/* #define CPLD_LANE_D_SEL	0x8 */

/* void board_config_lanes_mux(void) */
/* { */
/* 	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR; */
/* 	int srds_prtcl = (in_be32(&gur->rcwsr[4]) & */
/* 				FSL_CORENET_RCWSR4_SRDS_PRTCL) >> 26; */

/* 	u8 mux = 0; */
/* 	switch (srds_prtcl) { */
/* 	case 0x2: */
/* 	case 0x5: */
/* 	case 0x9: */
/* 	case 0xa: */
/* 	case 0xf: */
/* 		break; */
/* 	case 0x8: */
/* 		mux |= CPLD_LANE_C_SEL | CPLD_LANE_D_SEL; */
/* 		break; */
/* 	case 0x14: */
/* 		mux |= CPLD_LANE_A_SEL; */
/* 		break; */
/* 	case 0x17: */
/* 		mux |= CPLD_LANE_G_SEL; */
/* 		break; */
/* 	case 0x16: */
/* 	case 0x19: */
/* 	case 0x1a: */
/* 		mux |= CPLD_LANE_G_SEL | CPLD_LANE_C_SEL | CPLD_LANE_D_SEL; */
/* 		break; */
/* 	case 0x1c: */
/* 		mux |= CPLD_LANE_G_SEL | CPLD_LANE_A_SEL; */
/* 		break; */
/* 	default: */
/* 		printf("Fman:Unsupported SerDes Protocol 0x%02x\n", srds_prtcl); */
/* 		break; */
/* 	} */
/* 	CPLD_WRITE(serdes_mux, mux); */
/* } */

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	int flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash + PROMJET region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* if (flash_esel == -1) { */
	/* 	/\* very unlikely unless something is messed up *\/ */
	/* 	puts("Error: Could not find TLB for FLASH BASE\n"); */
	/* 	flash_esel = 2;	/\* give our best effort to continue *\/ */
	/* } else { */
		/* invalidate existing TLB entry for flash + promjet */
		disable_tlb(flash_esel);
	/* } */

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, flash_esel, BOOKE_PAGESZ_256M, 1);

	/* board_config_lanes_mux(); */

	return 0;
}

unsigned long get_board_sys_clk(unsigned long dummy)
{
	/* u8 sysclk_conf = CPLD_READ(sysclk_sw1); */

	/* switch (sysclk_conf & 0x7) { */
	/* case CPLD_SYSCLK_83: */
	/* 	return 83333333; */
	/* case CPLD_SYSCLK_100: */
		/* return 100000000; */
	/* default: */
		return 66666666;
	/* } */
}

#define NUM_SRDS_BANKS	2

int misc_init_r(void)
{
	serdes_corenet_t *regs = (void *)CONFIG_SYS_FSL_CORENET_SERDES_ADDR;
	u32 actual[NUM_SRDS_BANKS];
	unsigned int i;
	u8 sw;
	static const int freq[][3] = {
		{0, SRDS_PLLCR0_RFCK_SEL_100, SRDS_PLLCR0_RFCK_SEL_125},
		{SRDS_PLLCR0_RFCK_SEL_100, SRDS_PLLCR0_RFCK_SEL_156_25,
			SRDS_PLLCR0_RFCK_SEL_125}
	};
	/* sw = in_8(&CPLD_SW(2)) >> 2; */
	sw = 1 ;
	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		unsigned int clock = (sw >> (2 * i)) & 3;
		if (clock == 0x3) {
			printf("Warning: SDREFCLK%u switch setting of '11' is "
			       "unsupported\n", i + 1);
			break;
		}
		if (i == 0 && clock == 0)
			puts("Warning: SDREFCLK1 switch setting of"
				"'00' is unsupported\n");
		else
			actual[i] = freq[i][clock];

		/*
		 * PC board uses a different CPLD with PB board, this CPLD
		 * has cpld_ver_sub = 1, and pcba_ver = 5. But CPLD on PB
		 * board has cpld_ver_sub = 0, and pcba_ver = 4.
		 */
		/* if ((i == 1) && (CPLD_READ(cpld_ver_sub) == 1) && */
		/*     (CPLD_READ(pcba_ver) == 5)) { */
		/* 	/\* PC board bank2 frequency *\/ */
		/* 	actual[i] = freq[i-1][clock]; */
		/* } */
	}

	for (i = 0; i < NUM_SRDS_BANKS; i++) {
		u32 expected = in_be32(&regs->bank[i].pllcr0);
		expected &= SRDS_PLLCR0_RFCK_SEL_MASK;
		if (expected != actual[i]) {
			printf("Warning: SERDES bank %u expects reference clock"
			       " %sMHz, but actual is %sMHz\n", i + 1,
			       serdes_clock_to_string(expected),
			       serdes_clock_to_string(actual[i]));
		}
	}

  /*dereset lan and pcie*/
  upcpld_write(0x54, 0x04);
  upcpld_write(0x56, 0x80);
	return 0;
}

int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = getenv_bootm_low();
	size = getenv_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#if defined(CONFIG_HAS_FSL_DR_USB) || defined(CONFIG_HAS_FSL_MPH_USB)
	fdt_fixup_dr_usb(blob, bd);
#endif

#ifdef CONFIG_PCI
	pci_of_setup(blob, bd);
#endif

	fdt_fixup_liodn(blob);
#ifdef CONFIG_SYS_DPAA_FMAN
	/* fdt_fixup_fman_ethernet(blob); */
#endif

	return 0;
}
int do_netinit (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
  unsigned char rdData;
  /* MV Switch reset_n is upcpld gpio 17. */
  /* Configure it as output, pulse the reset signal. */
  rdData = upcpld_read (0x42);
  upcpld_write(0x42, (rdData | 0x80));
  rdData = upcpld_read (0x56);
  /* activate reset */
  upcpld_write(0x56, (rdData & 0x7F));
  udelay(500000);
  /* deactivate reset */
  upcpld_write(0x56, (rdData | 0x80));
  udelay(500000);

  /* Initialize the Marvell switch */

  /* mvswitch_init(); */
  return 0;
}

U_BOOT_CMD(netinit, 1, 0, do_netinit,
           "initialize network switch", "");
int do_pciinit (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
  pci_init_board();

  return 0;
}

U_BOOT_CMD(pciinit, 1, 0, do_pciinit,
           "re-enumerate PCIe busses", "");

