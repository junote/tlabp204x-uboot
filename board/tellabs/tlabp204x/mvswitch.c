
#include <common.h>
#include <command.h>
#include "config.h"
#include <asm/io.h>
#include <stdarg.h>
#include <fm_eth.h>
#include <fsl_mdio.h>
#include <fsl_dtsec.h>
#include "phy.h"
#include <mvswitch.h>


unsigned short mvswitch_read (unsigned char devad, unsigned char regad)
{
    struct mii_dev *fsl_mdio;
    int val;
    int timeout;

    fsl_mdio = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);

    tsec_phy_write(fsl_mdio, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR, MDIO_DEVAD_NONE, 0,
		   (0x9800 | ((devad & 0x1F) << 5) | (regad & 0x1F)));

    timeout = 1000;
    do {
      val = tsec_phy_read(fsl_mdio, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR, MDIO_DEVAD_NONE, 0);
      timeout--;
    } while (((val & 0x8000) != 0) && (timeout > 0));

    val = tsec_phy_read(fsl_mdio, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR, MDIO_DEVAD_NONE, 1);

    return ((unsigned short) (val & 0x0000FFFF));
}

void mvswitch_write (unsigned char devad, unsigned char regad, unsigned short wrdata)
{
    struct mii_dev *fsl_mdio;
    int val;
    int timeout;

    fsl_mdio = miiphy_get_dev_by_name(DEFAULT_FM_MDIO_NAME);

    tsec_phy_write(fsl_mdio, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR, MDIO_DEVAD_NONE, 1,
		     ((int) wrdata));
    tsec_phy_write(fsl_mdio, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR, MDIO_DEVAD_NONE, 0,
		     (0x9400 | ((devad & 0x1F) << 5) | (regad & 0x1F)));

    timeout = 1000;
    do {
      val = tsec_phy_read(fsl_mdio, CONFIG_SYS_FM1_DTSEC1_PHY_ADDR, MDIO_DEVAD_NONE, 0);
      timeout--;
    } while (((val & 0x8000) != 0) && (timeout > 0));
}

void mvswitch_setbits (unsigned char devad, unsigned char regad, unsigned short wrmask)
{
  unsigned short rddata;
  rddata = mvswitch_read (devad, regad);
  mvswitch_write (devad, regad, (rddata | wrmask));
}

void mvswitch_clearbits (unsigned char devad, unsigned char regad, unsigned short wrmask)
{
  unsigned short rddata;
  rddata = mvswitch_read (devad, regad);
  mvswitch_write (devad, regad, (rddata & ~wrmask));
}

unsigned short mvswitch_phyread (unsigned char devad, unsigned char regad)
{
    int val;
    int timeout;

    mvswitch_write(0x1C, 0x18, (0x9800 | ((devad & 0x1F) << 5) | (regad & 0x1F)));

    timeout = 1000;
    do {
      val = mvswitch_read(0x1C, 0x18);
      timeout--;
    } while (((val & 0x8000) != 0) && (timeout > 0));

    val = mvswitch_read(0x1C, 0x19);

    return ((unsigned short) (val & 0x0000FFFF));
}

//@@@@@@@@@@@
void mvswitch_phywrite (unsigned char devad, unsigned char regad, unsigned short wrdata)
{
    int val;
    int timeout;

    mvswitch_write(0x1C, 0x19, wrdata);

    mvswitch_write(0x1C, 0x18, (0x9400 | ((devad & 0x1F) << 5) | (regad & 0x1F)));

    timeout = 1000;
    do {
      val = mvswitch_read(0x1C, 0x18);
      timeout--;
    } while (((val & 0x8000) != 0) && (timeout > 0));
}

void mvswitch_physetbits (unsigned char devad, unsigned char regad, unsigned short wrmask)
{
  unsigned short rddata;
  rddata = mvswitch_phyread (devad, regad);
  mvswitch_phywrite (devad, regad, (rddata | wrmask));
}

void mvswitch_phyclearbits (unsigned char devad, unsigned char regad, unsigned short wrmask)
{
  unsigned short rddata;
  rddata = mvswitch_phyread (devad, regad);
  mvswitch_phywrite (devad, regad, (rddata & ~wrmask));
}

unsigned short mvswitch_poll (unsigned char devad, unsigned char regad,
			      unsigned short rdmask,
			      unsigned short expValue, unsigned int loopmax,
			      int dly_us)
{
  unsigned short readVal, retVal;
  unsigned long loopcnt;
  loopcnt = 0;
  retVal = ~expValue;
  while (loopcnt < loopmax) {
    readVal = mvswitch_read (devad, regad);
    if ((readVal & rdmask) == expValue) { retVal = expValue; break; }
    udelay(dly_us);
    loopcnt++;
  }
  return retVal;
}

unsigned short mvswitch_phypoll (unsigned char devad, unsigned char regad,
			      unsigned short rdmask,
			      unsigned short expValue, unsigned int loopmax,
			      int dly_us)
{
  unsigned short readVal, retVal;
  unsigned long loopcnt;
  loopcnt = 0;
  retVal = ~expValue;
  while (loopcnt < loopmax) {
    readVal = mvswitch_phyread (devad, regad);
    if ((readVal & rdmask) == expValue) { retVal = expValue; break; }
    udelay(dly_us);
    loopcnt++;
  }
  return retVal;
}

/*****************************************************************************/
/** mvswitch_init
 *
 *****************************************************************************/
void mvswitch_init( void )
{
    char *var;

    /* if ((var = getenv("netinitcmd")) != NULL) { */
       /* printf ("Executing \"netinitcmd\" to initialize switch. May take a while...\n"); */
       printf ("initialize switch. May take a while...\n");
       /* run_command (getenv ("netinitcmd"), CMD_FLAG_BOOTD); */
       mvswitch_phywrite(0,0,0x3100);
       mvswitch_phywrite(1,0,0x3100);
       mvswitch_phywrite(2,0,0x3100);

       mvswitch_write(0x18,1,0x403e);
       mvswitch_write(0x19,1,0x403e);
       mvswitch_write(0x1a,1,0x403e);

       mvswitch_write(0x10,4,0x7f);
       mvswitch_write(0x11,4,0x7f);
       mvswitch_write(0x12,4,0x7f);
       mvswitch_write(0x18,4,0x7f);
       mvswitch_write(0x19,4,0x7f);
       mvswitch_write(0x1a,4,0x7f);
    /* } else { */
      /* printf ("Missing \"netinitcmd\" env var. Setup this var to load and exec Fulcrum script.\n"); */
    /* } */

}   /* end mvswitch_switch_init */

int do_mvswitch (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
        unsigned char devad;
        unsigned char regad;
        unsigned short data;
	unsigned short mask;
	unsigned short rdData;
	unsigned long loopmax;
	unsigned long intrval;
        unsigned long i;

	if (argc >= 2) {
          if ((!strcmp(argv[1], "r")) && (argc >= 4)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            i = 1;
            if (argc >= 5) {
              i = simple_strtoul(argv[4], NULL, 10);
              if (i < 1) { i = 1; }
              if (i > 31) { i = 31; }
            }
            while (i > 0) {
	      data = mvswitch_read (devad, regad);
              printf ("[%x.%02x] : 0x%04x\n", devad, regad, data);
              regad = regad + 1;
	      if (regad >= 32) { regad = 0; }
              i--;
            }
          } else if ((!strcmp(argv[1], "pr")) && (argc >= 4)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            i = 1;
            if (argc >= 5) {
              i = simple_strtoul(argv[4], NULL, 10);
              if (i < 1) { i = 1; }
              if (i > 31) { i = 31; }
            }
            while (i > 0) {
	      data = mvswitch_phyread (devad, regad);
              printf ("[%x.%02x] : 0x%04x\n", devad, regad, data);
              regad = regad + 1;
	      if (regad >= 32) { regad = 0; }
              i--;
            }
          } else if ((!strcmp(argv[1], "w")) && (argc == 5)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
	    mvswitch_write (devad, regad, data);
          } else if ((!strcmp(argv[1], "pw")) && (argc == 5)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
	    mvswitch_phywrite (devad, regad, data);
          } else if ((!strcmp(argv[1], "poll")) && (argc == 8)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            mask = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[5], NULL, 16));
            loopmax = simple_strtoul(argv[6], NULL, 16);
            intrval = simple_strtoul(argv[7], NULL, 16);
	    rdData = mvswitch_poll (devad, regad, mask, data, loopmax, intrval);
	    if (rdData != data) { printf ("(*) mvswitch_poll failed 0x%04x\n", data); }
          } else if ((!strcmp(argv[1], "ppoll")) && (argc == 8)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            mask = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[5], NULL, 16));
            loopmax = simple_strtoul(argv[6], NULL, 16);
            intrval = simple_strtoul(argv[7], NULL, 16);
	    rdData = mvswitch_phypoll (devad, regad, mask, data, loopmax, intrval);
	    if (rdData != data) { printf ("(*) mvswitch_phypoll failed 0x%04x\n", data); }
          } else if (((!strcmp(argv[1], "set"))) && (argc == 5)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
	    mvswitch_setbits (devad, regad, data);
	    data = mvswitch_read (devad, regad);
	    printf ("[%x.%02x] : 0x%04x\n", devad, regad, data);
          } else if (((!strcmp(argv[1], "pset"))) && (argc == 5)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
	    mvswitch_physetbits (devad, regad, data);
	    data = mvswitch_phyread (devad, regad);
	    printf ("[%x.%02x] : 0x%04x\n", devad, regad, data);
          } else if (((!strcmp(argv[1], "clr"))) && (argc == 5)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
	    mvswitch_clearbits (devad, regad, data);
	    data = mvswitch_read (devad, regad);
	    printf ("[%x.%02x] : 0x%04x\n", devad, regad, data);
          } else if (((!strcmp(argv[1], "pclr"))) && (argc == 5)) {
            devad = (0x00FF & simple_strtoul(argv[2], NULL, 16));
            regad = (0x00FF & simple_strtoul(argv[3], NULL, 16));
            data = (0xFFFF & simple_strtoul(argv[4], NULL, 16));
	    mvswitch_phyclearbits (devad, regad, data);
	    data = mvswitch_phyread (devad, regad);
	    printf ("[%x.%02x] : 0x%04x\n", devad, regad, data);
          } else {
            printf("usage -- mvswitch r|w [devad] [regad] [data]      : read/write switch/port (devad 0x10-0x1c) regs\n");
            printf("usage -- mvswitch pr|pw [devad] [regad] [data]    : read/write internal phy (devad 0x00-0x07) regs\n");
            printf("usage -- mvswitch set|clr [devad] [regad] [mask]  : set/clr switch/port (devad 0x10-0x1c) bits \n");
            printf("usage -- mvswitch pset|pclr [devad] [regad] [mask]  : set/clr phy (devad 0x00-0x07) bits \n");
            printf("usage -- mvswitch poll [devad] [regad] [mask] [expVal] [loopmax] [intrvl_us]  : poll switch/port reg\n");
            printf("usage -- mvswitch ppoll [devad] [regad] [mask] [expVal] [loopmax] [intrvl_us]  : poll internal phy reg\n");
          }
        } else {
            printf("usage -- mvswitch r|w [devad] [regad] [data]      : read/write switch/port (devad 0x10-0x1c) regs\n");
            printf("usage -- mvswitch pr|pw [devad] [regad] [data]    : read/write internal phy (devad 0x00-0x07) regs\n");
            printf("usage -- mvswitch set|clr [devad] [regad] [mask]  : set/clr switch/port (devad 0x10-0x1c) bits \n");
            printf("usage -- mvswitch pset|pclr [devad] [regad] [mask]  : set/clr phy (devad 0x00-0x07) bits \n");
            printf("usage -- mvswitch poll [devad] [regad] [mask] [expVal] [loopmax] [intrvl_us]  : poll switch/port reg\n");
            printf("usage -- mvswitch ppoll [devad] [regad] [mask] [expVal] [loopmax] [intrvl_us]  : poll internal phy reg\n");
        }
        return 0;
}

U_BOOT_CMD(
	mvswitch, CONFIG_SYS_MAXARGS, 1, do_mvswitch,
	"mvswitch    - read or write the Marvell switch", ""
);

