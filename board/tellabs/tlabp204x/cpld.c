#include <common.h>
#include <command.h>
#include "config.h"
#include <asm/io.h>

#include <stdarg.h>

unsigned char upcpld_read (unsigned long address)
{
  volatile unsigned char * addr;
  addr = ((unsigned char *) (UPCPLD_BASE + address));
  return *addr;
  return 0;
}

void upcpld_write (unsigned long address, unsigned char wrdata)
{
  volatile unsigned char * addr;
  addr = ((unsigned char *) (UPCPLD_BASE + address));
  *addr = wrdata;
}

unsigned char mbcpld_read (unsigned long address)
{
  volatile unsigned char * addr;
  addr = ((unsigned char *) (MBCPLD_BASE + address));
  return *addr;
  return 0;
}

void mbcpld_write (unsigned long address, unsigned char wrdata)
{
  volatile unsigned char * addr;
  addr = ((unsigned char *) (MBCPLD_BASE + address));
  *addr = wrdata;
}

/* unsigned short spmxfpga_read (unsigned long address) */
/* { */
/*   volatile unsigned short * addr; */
/*   addr = ((unsigned short *) (SPMXFPGA_BASE + address)); */
/*   return *addr; */
/*   return 0; */
/* } */

/* void spmxfpga_write (unsigned long address, unsigned short wrdata) */
/* { */
/*   volatile unsigned short * addr; */
/*   addr = ((unsigned short *) (SPMXFPGA_BASE + address)); */
/*   *addr = wrdata; */
/* } */

void cpld_setgpio(int gpionum, int pdir, int podr, int pdat)
{
  unsigned long pdir_addr, podr_addr, pdat_addr;
  unsigned char regdata;
  if ((gpionum > 0) && (gpionum < 31)) {
    pdir_addr = ((gpionum < 9)  ? 0x40 : (gpionum < 17) ? 0x41 : 
		 (gpionum < 25) ? 0x42 : 0x43);
    podr_addr = ((gpionum < 9)  ? 0x44 : (gpionum < 17) ? 0x45 : 
		 (gpionum < 25) ? 0x46 : 0x47);
    pdat_addr = ((gpionum < 9)  ? 0x54 : (gpionum < 17) ? 0x55 : 
		 (gpionum < 25) ? 0x56 : 0x57);

    regdata = upcpld_read(pdir_addr);
    if (pdir > 0) {
      upcpld_write (pdir_addr, (regdata | (0x80 >> ((gpionum - 1) % 8))));
    } else {
      upcpld_write (pdir_addr, (regdata & ~(0x80 >> ((gpionum - 1) % 8))));
    }

    regdata = upcpld_read(podr_addr);
    if (podr > 0) {
      upcpld_write (podr_addr, (regdata | (0x80 >> ((gpionum - 1) % 8))));
    } else {
      upcpld_write (podr_addr, (regdata & ~(0x80 >> ((gpionum - 1) % 8))));
    }

    regdata = upcpld_read(pdat_addr);
    if (pdat > 0) {
      upcpld_write (pdat_addr, (regdata | (0x80 >> ((gpionum - 1) % 8))));
    } else {
      upcpld_write (pdat_addr, (regdata & ~(0x80 >> ((gpionum - 1) % 8))));
    }
  }
}

void cpld_getgpio(int gpionum, int *pdir, int *podr, int *pdat)
{
  unsigned long pdir_addr, podr_addr, pdat_addr;
  unsigned char regdata;
  if ((gpionum > 0) && (gpionum < 31)) {
    pdir_addr = ((gpionum < 9)  ? 0x40 : (gpionum < 17) ? 0x41 : 
		 (gpionum < 25) ? 0x42 : 0x43);
    podr_addr = ((gpionum < 9)  ? 0x44 : (gpionum < 17) ? 0x45 : 
		 (gpionum < 25) ? 0x46 : 0x47);
    pdat_addr = ((gpionum < 9)  ? 0x54 : (gpionum < 17) ? 0x55 : 
		 (gpionum < 25) ? 0x56 : 0x57);

    regdata = upcpld_read(pdir_addr);
    *pdir = ((regdata | (0x80 >> ((gpionum - 1) % 8))) == 0) ? 0 : 1;
    regdata = upcpld_read(podr_addr);
    *podr = ((regdata | (0x80 >> ((gpionum - 1) % 8))) == 0) ? 0 : 1;
    regdata = upcpld_read(pdat_addr);
    *pdat = ((regdata | (0x80 >> ((gpionum - 1) % 8))) == 0) ? 0 : 1;
  }
}

int do_upcpld (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        unsigned long addr;
        unsigned char data;
        unsigned char gpiopin;
        unsigned long i;

	if (argc >= 2) {
          if ((!strcmp(argv[1], "gpio")) && (argc == 3)) {
            gpiopin = simple_strtoul(argv[2], NULL, 10);
            if ((gpiopin > 0) && (gpiopin <= 30)) {
              addr = ((gpiopin <= 8) ? 0x00 : (gpiopin <= 16) ? 0x01 : (gpiopin <= 24) ? 0x02 : 0x03);
              data = 0x80 >> ((gpiopin-1) % 8);
               printf ("upcpld_gpio_%d: dir = %s, ", gpiopin, ((upcpld_read(0x40+addr) & data) ? "out" : "in"));
                if ((upcpld_read(0x44+addr) & data) && (upcpld_read(0x40+addr) & data)) {
                  printf ("open-drain, ");
                }
                printf ("val = %s\n", ((upcpld_read(0x54+addr) & data) ? "1" : "0"));
            } else {
                printf ("usage -- upcpld gpio <1-30> [<in|out|odr>] [<0|1>] \n");
            }
          } else if ((!strcmp(argv[1], "gpio")) && (argc > 3)) {
            gpiopin = simple_strtoul(argv[2], NULL, 10);
            if ((gpiopin > 0) && (gpiopin <= 30)) {
              addr = ((gpiopin <= 8) ? 0x00 : (gpiopin <= 16) ? 0x01 : (gpiopin <= 24) ? 0x02 : 0x03);
              data = 0x80 >> ((gpiopin-1) % 8);

              if (!strcmp(argv[3], "in")) {
                cpld_setgpio(gpiopin, 0, 0, 0);
              } else if ((!strcmp(argv[3], "out")) && (argc > 4)) {
                if (!strcmp(argv[4], "0")) {
                  cpld_setgpio(gpiopin, 1, 0, 0);
                } else if (!strcmp(argv[4], "1")) {
                 cpld_setgpio(gpiopin, 1, 0, 1);
                } else {
                  printf ("usage -- upcpld gpio <1-30> [<in|out|odr>] [<0|1>] \n");
                }
              } else if ((!strcmp(argv[3], "odr")) && (argc > 4)) {
                if (!strcmp(argv[4], "0")) {
                  cpld_setgpio(gpiopin, 1, 1, 0);
                } else if (!strcmp(argv[4], "1")) {
                 cpld_setgpio(gpiopin, 1, 1, 1);
                } else {
                  printf ("usage -- upcpld gpio <1-30> [<in|out|odr>] [<0|1>] \n");
                }
              }

            } else {
                printf ("usage -- upcpld gpio <1-30> [<in|out|odr>] [<0|1>] \n");
            }

          } else if ((!strcmp(argv[1], "r")) && (argc >= 3)) {
            addr = simple_strtoul(argv[2], NULL, 16);
            i = 1;
            if (argc >= 4) {
              i = simple_strtoul(argv[3], NULL, 10);
              if (i < 1) { i = 1; }
              if (i > 255) { i = 255; }
            }
            while (i > 0) {
              data = upcpld_read (addr);
              printf ("[0x%02x] : 0x%02x\n", ((unsigned int) addr), 
		      ((unsigned int) data));
              addr++;
              i--;
            }
          }
          else if ((!strcmp(argv[1], "w")) && (argc == 4)) {
            addr = simple_strtoul(argv[2], NULL, 16);
            data = simple_strtoul(argv[3], NULL, 16);
            upcpld_write (addr, data);
            printf ("[0x%02x] : 0x%02x\n", ((unsigned int) addr), 
		    ((unsigned int) data));
          } else {
            printf("usage -- upcpld r|w [addr] [data] \n");
            printf("usage -- upcpld gpio <1-30> [<in|out|odr>] [<0|1>] \n");
          }
        } else {
          printf("usage -- upcpld r|w [addr] [data] \n");
          printf("usage -- upcpld gpio <1-30> [<in|out|odr>] [<0|1>] \n");
        }
        return 0;
}

int do_mbcpld (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        unsigned long addr;
        unsigned char data;
        unsigned long i;

	if (argc >= 2) {
          if ((!strcmp(argv[1], "r")) && (argc >= 3)) {
            addr = simple_strtoul(argv[2], NULL, 16);
            i = 1;
            if (argc >= 4) {
              i = simple_strtoul(argv[3], NULL, 10);
              if (i < 1) { i = 1; }
              if (i > 255) { i = 255; }
            }
            while (i > 0) {
              data = mbcpld_read (addr);
              printf ("[0x%02x] : 0x%02x\n", ((unsigned int) addr), 
		      ((unsigned int) data));
              addr++;
              i--;
            }
          }
          else if ((!strcmp(argv[1], "w")) && (argc == 4)) {
            addr = simple_strtoul(argv[2], NULL, 16);
            data = simple_strtoul(argv[3], NULL, 16);
            mbcpld_write (addr, data);
            printf ("[0x%02x] : 0x%02x\n", ((unsigned int) addr), 
		    ((unsigned int) data));
          } 
          else {
            printf("usage -- mbcpld r|w [addr] [data] \n");
          }
        } else {
          printf("usage -- mbcpld r|w [addr] [data] \n");
        }
        return 0;
}

/* int do_spmxfpga (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]) */
/* { */
/*         unsigned long addr; */
/*         unsigned short data; */
/*         unsigned long i; */

/* 	if (argc >= 2) { */
/*           if ((!strcmp(argv[1], "r")) && (argc >= 3)) { */
/*             addr = simple_strtoul(argv[2], NULL, 16); */
/*             i = 1; */
/*             if (argc >= 4) { */
/*               i = simple_strtoul(argv[3], NULL, 10); */
/*               if (i < 1) { i = 1; } */
/*               if (i > 255) { i = 255; } */
/*             } */
/*             while (i > 0) { */
/*               data = spmxfpga_read (addr); */
/*               printf ("[0x%04x] : 0x%04x\n", ((unsigned int) addr),  */
/* 		      ((unsigned int) data)); */
/*               addr = addr + 2; */
/*               i--; */
/*             } */
/*           } */
/*           else if ((!strcmp(argv[1], "w")) && (argc == 4)) { */
/*             addr = simple_strtoul(argv[2], NULL, 16); */
/*             data = simple_strtoul(argv[3], NULL, 16); */
/*             spmxfpga_write (addr, data); */
/*             printf ("[0x%04x] : 0x%04x\n", ((unsigned int) addr),  */
/* 		    ((unsigned int) data)); */
/*           }  */
/*           else { */
/*             printf("usage -- spmxfpga r|w [addr] [data] \n"); */
/*           } */
/*         } else { */
/*           printf("usage -- spmxfpga r|w [addr] [data] \n"); */
/*         } */
/*         return 0; */
/* } */

int do_led (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        unsigned char data;

        if (argc == 2) {
          if (!strcmp(argv[1], "active")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x06);
#else
            data = mbcpld_read (0x04);
#endif
            printf ("%s ", (((data & 0x60) == 0x60) ? "amber" : 
                            ((data & 0x60) == 0x40) ? "red"   : 
                            ((data & 0x60) == 0x20) ? "green" : "off"));
            printf ("%s\n", ((data & 0x10) ? "blink" : ""));
          } else if (!strcmp(argv[1], "fault")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x06);
#else
            data = mbcpld_read (0x04);
#endif
            printf ("%s ", (((data & 0x06) == 0x06) ? "amber" : 
                            ((data & 0x06) == 0x04) ? "red"   : 
                            ((data & 0x06) == 0x02) ? "green" : "off"));
            printf ("%s\n", ((data & 0x01) ? "blink" : ""));
          } else if (!strcmp(argv[1], "sync")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x07);
#else
            data = mbcpld_read (0x05);
#endif
            printf ("%s ", (((data & 0x60) == 0x60) ? "amber" : 
                            ((data & 0x60) == 0x40) ? "red"   : 
                            ((data & 0x60) == 0x20) ? "green" : "off"));
            printf ("%s\n", ((data & 0x10) ? "blink" : ""));
          } else if (!strcmp(argv[1], "power")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x07);
#else
            data = mbcpld_read (0x05);
#endif
            if (data & 0x08) {
              printf ("hw-controlled\n");
            } else {
              printf ("%s ", (((data & 0x06) == 0x06) ? "amber" : 
                              ((data & 0x06) == 0x04) ? "red"   : 
                              ((data & 0x06) == 0x02) ? "green" : "off"));
              printf ("%s\n", ((data & 0x01) ? "blink" : ""));
            }
          } else {
            printf("usage -- led <active|fault|power|sync> [off|red|green|amber|hw] [blink] \n");
          }
        } else if (argc > 2) {
          if (!strcmp(argv[1], "active")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x06) & 0x0F;
#else
            data = mbcpld_read (0x04) & 0x0F;
#endif
            if (argc > 3) {
              if (!strcmp(argv[3], "blink")) {
                data |= 0x10;
              }
            }
            data |= ((!strcmp(argv[2], "amber")) ? 0x60 : 
                     (!strcmp(argv[2], "red"))   ? 0x40 : 
                     (!strcmp(argv[2], "green")) ? 0x20 : 0x00);
#ifdef CONFIG_OADMRS20
            upcpld_write (0x06, data);
#else
            mbcpld_write (0x04, data);
#endif
          } else if (!strcmp(argv[1], "fault")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x06) & 0xF0;
#else
            data = mbcpld_read (0x04) & 0xF0;
#endif
            if (argc > 3) {
              if (!strcmp(argv[3], "blink")) {
                data |= 0x01;
              }
            }
            data |= ((!strcmp(argv[2], "amber")) ? 0x06 : 
                     (!strcmp(argv[2], "red"))   ? 0x04 : 
                     (!strcmp(argv[2], "green")) ? 0x02 : 0x00);
#ifdef CONFIG_OADMRS20
            upcpld_write (0x06, data);
#else
            mbcpld_write (0x04, data);
#endif
          } else if (!strcmp(argv[1], "sync")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x07) & 0x0F;
#else
            data = mbcpld_read (0x05) & 0x0F;
#endif
            if (argc > 3) {
              if (!strcmp(argv[3], "blink")) {
                data |= 0x10;
              }
            }
            data |= ((!strcmp(argv[2], "amber")) ? 0x60 : 
                     (!strcmp(argv[2], "red"))   ? 0x40 : 
                     (!strcmp(argv[2], "green")) ? 0x20 : 0x00);
#ifdef CONFIG_OADMRS20
            upcpld_write (0x07, data);
#else
            mbcpld_write (0x05, data);
#endif
          } else if (!strcmp(argv[1], "power")) {
#ifdef CONFIG_OADMRS20
            data = upcpld_read (0x07) & 0xF0;
#else
            data = mbcpld_read (0x05) & 0xF0;
#endif
            if (!strcmp(argv[2], "hw")) {
#ifdef CONFIG_OADMRS20
              upcpld_write (0x07, (data & 0xF7));
#else
              mbcpld_write (0x05, (data & 0xF7));
#endif
            } else {
              if (argc > 3) {
                if (!strcmp(argv[3], "blink")) {
                  data |= 0x01;
                }
              }
              data |= ((!strcmp(argv[2], "amber")) ? 0x06 : 
                       (!strcmp(argv[2], "red"))   ? 0x04 : 
                       (!strcmp(argv[2], "green")) ? 0x02 : 0x00);
#ifdef CONFIG_OADMRS20
              upcpld_write (0x07, data);
#else
              mbcpld_write (0x05, data);
#endif
            }
          } else {
            printf("usage -- led <active|fault|power|sync> [off|red|green|amber|hw] [blink] \n");
          }
        } else {
            printf("usage -- led <active|fault|power|sync> [off|red|green|amber|hw] [blink] \n");
        }
        return 0;
}

U_BOOT_CMD(
	upcpld, CONFIG_SYS_MAXARGS, 1, do_upcpld,
	"upcpld    - read or write the proc card CPLD.", ""
);

U_BOOT_CMD(
	mbcpld, CONFIG_SYS_MAXARGS, 1, do_mbcpld,
	"mbcpld    - read or write the motherboard CPLD.", ""
);

/* U_BOOT_CMD( */
/* 	spmxfpga, CONFIG_SYS_MAXARGS, 1, do_spmxfpga, */
/* 	"spmxfpga    - read or write the SPMX FPGA.", "" */
/* ); */

U_BOOT_CMD(
	led, CONFIG_SYS_MAXARGS, 1, do_led,
	"led    - front panel leds.", ""
);
