/*
 * Copyright 2008-2011 Freescale Semiconductor, Inc.
 *
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/mmu.h>

struct fsl_e_tlb_entry tlb_table[] = {
	/* TLB 0 - for temp stack in cache */
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 4 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 4 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 8 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 8 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),
	SET_TLB_ENTRY(0, CONFIG_SYS_INIT_RAM_ADDR + 12 * 1024,
		      CONFIG_SYS_INIT_RAM_ADDR_PHYS + 12 * 1024,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	SET_TLB_ENTRY(0, UPCPLD_BASE, UPCPLD_BASE_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	SET_TLB_ENTRY(0, MBCPLD_BASE, MBCPLD_BASE_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_4K, 0),

	//	SET_TLB_ENTRY(0, SPMXFPGA_BASE, SPMXFPGA_BASE_PHYS,
	//		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
	//		      0, 0, BOOKE_PAGESZ_4K, 0),

	/* TLB 1 */
	/* *I*** - Covers boot page */
#if defined(CONFIG_SYS_RAMBOOT) && defined(CONFIG_SYS_INIT_L3_ADDR)
	/*
	 * *I*G - L3SRAM. When L3 is used as 1M SRAM, the address of the
	 * SRAM is at 0xfff00000, it covered the 0xfffff000.
	 */
	SET_TLB_ENTRY(1, CONFIG_SYS_INIT_L3_ADDR, CONFIG_SYS_INIT_L3_ADDR,
			MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
			0, 0, BOOKE_PAGESZ_1M, 1),
#else
	SET_TLB_ENTRY(1, 0xfffff000, 0xfffff000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 0, BOOKE_PAGESZ_4K, 1),
#endif

	/* *I*G* - CCSRBAR */
	SET_TLB_ENTRY(1, CONFIG_SYS_CCSRBAR, CONFIG_SYS_CCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 1, BOOKE_PAGESZ_16M, 1),

	/* *I*G* - Flash, localbus */
	/* This will be changed to *I*G* after relocation to RAM. */
	SET_TLB_ENTRY(1, CONFIG_SYS_FLASH_BASE, CONFIG_SYS_FLASH_BASE_PHYS,
		      MAS3_SX|MAS3_SR, MAS2_W|MAS2_G,
		      0, 2, BOOKE_PAGESZ_256M, 1),

	/* *I*G* - PCI */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT, CONFIG_SYS_PCIE1_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 3, BOOKE_PAGESZ_256M, 1),

	/* *I*G* - PCI */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT + 0x20000000,
		      CONFIG_SYS_PCIE1_MEM_PHYS + 0x20000000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 4, BOOKE_PAGESZ_256M, 1),

	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_MEM_VIRT + 0x40000000,
#ifdef CONFIG_BIGPCIE2SPACE
		      CONFIG_SYS_PCIE1_MEM_PHYS + 0xA0000000,
#else
		      CONFIG_SYS_PCIE1_MEM_PHYS + 0x40000000,
#endif
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 5, BOOKE_PAGESZ_256M, 1),

	/* *I*G* - PCI I/O */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE1_IO_VIRT, CONFIG_SYS_PCIE1_IO_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 6, BOOKE_PAGESZ_256K, 1),

	/* *I*G* - PCI */
	/* SET_TLB_ENTRY(1, SPMXFPGA_BASE, SPMXFPGA_BASE_PHYS, */
	/* 	      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G, */
	/* 	      0, 7, BOOKE_PAGESZ_64M, 1), */

	/* *I*G* - PCI I/O */
	SET_TLB_ENTRY(1, CONFIG_SYS_PCIE2_IO_VIRT, CONFIG_SYS_PCIE2_IO_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 8, BOOKE_PAGESZ_64K, 1),

	/* Bman/Qman */
#ifdef CONFIG_SYS_BMAN_MEM_PHYS
	SET_TLB_ENTRY(1, CONFIG_SYS_BMAN_MEM_BASE, CONFIG_SYS_BMAN_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 9, BOOKE_PAGESZ_1M, 1),
	SET_TLB_ENTRY(1, CONFIG_SYS_BMAN_MEM_BASE + 0x00100000,
		      CONFIG_SYS_BMAN_MEM_PHYS + 0x00100000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 10, BOOKE_PAGESZ_1M, 1),
#endif
#ifdef CONFIG_SYS_QMAN_MEM_PHYS
	SET_TLB_ENTRY(1, CONFIG_SYS_QMAN_MEM_BASE, CONFIG_SYS_QMAN_MEM_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, 0,
		      0, 11, BOOKE_PAGESZ_1M, 1),
	SET_TLB_ENTRY(1, CONFIG_SYS_QMAN_MEM_BASE + 0x00100000,
		      CONFIG_SYS_QMAN_MEM_PHYS + 0x00100000,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 12, BOOKE_PAGESZ_1M, 1),
#endif
#ifdef CONFIG_SYS_DCSRBAR_PHYS
	SET_TLB_ENTRY(1, CONFIG_SYS_DCSRBAR, CONFIG_SYS_DCSRBAR_PHYS,
		      MAS3_SX|MAS3_SW|MAS3_SR, MAS2_I|MAS2_G,
		      0, 13, BOOKE_PAGESZ_4M, 1),
#endif

};

int num_tlb_entries = ARRAY_SIZE(tlb_table);
