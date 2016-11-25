/* #ifndef __TLABP204X_CPLD_H */
/* #define __TLABP204X_CPLD_H */

unsigned char upcpld_read (unsigned long address);
void upcpld_write (unsigned long address, unsigned char wrdata);
unsigned char mbcpld_read (unsigned long address);
void mbcpld_write (unsigned long address, unsigned short wrdata);
unsigned short spmxfpga_read (unsigned long address);
void spmxfpga_write (unsigned long address, unsigned short wrdata);

void cpld_setgpio(int gpionum, int pdir, int podr, int psor, int pdat);
void cpld_getgpio(int gpionum, int *pdir, int *podr, int *psor, int *pdat);

int do_upcpld (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_mbcpld (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);
int do_spmxfpga (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

/* #endif /\* __TLABP204X_CPLD_H *\/ */
