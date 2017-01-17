/*
 * Copyright (c) 2009 Tellabs Inc.
 * Copyright (c) 2006 Ben Warren, Qstreams Networks Inc.
 * With help from the common/soft_spi and cpu/mpc8260 drivers
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
#include <config.h>
#include <asm/types.h>
#include <malloc.h>
#include <spi.h>
#include <tlab_cpld.h>

typedef struct spi8xxx {
        u32 mode;       /* mode register  */
        u32 event;      /* event register */
        u32 mask;       /* mask register  */
        u32 com;        /* command register */
        u32 tx;         /* transmit register */
        u32 rx;         /* receive register */
#ifdef CONFIG_OSM4F
        u16 cs;         /* chip-select select register */
#endif
} spi8xxx_t;

/* PSM-1S */
#define CPLD_SPI_BUS_0_OFFSET (0x000000C0)
#define CPLD_SPI_BUS_1_OFFSET (0x000000E0)

/* OSM2C */
#define CPLD_SPI_BUS_2_OFFSET (0x00000100)
#define CPLD_SPI_BUS_3_OFFSET (0x00000120)
#define CPLD_SPI_BUS_4_OFFSET (0x00000140)
#define CPLD_SPI_BUS_5_OFFSET (0x00000160)
#define CPLD_SPI_BUS_6_OFFSET (0x00000180)

/* OADMRS20 */
#define CPLD_SPI_BUS_7_OFFSET (0x00000100)

#define NUM_BUSES 7

#ifdef CONFIG_OSM4F
#define NUM_CHIPSELECTS 15
#else
#define NUM_CHIPSELECTS 0
#endif

#define SPI_EV_NE	(0x80000000 >> 22)	/* Receiver Not Empty */
#define SPI_EV_NF	(0x80000000 >> 23)	/* Transmitter Not Full */

#define SPI_MODE_LOOP	(0x80000000 >> 1)	/* Loopback mode */
#define SPI_MODE_REV	(0x80000000 >> 5)	/* Reverse mode - MSB first */
#define SPI_MODE_MS	(0x80000000 >> 6)	/* Always master */
#define SPI_MODE_EN	(0x80000000 >> 7)	/* Enable interface */

#define SPI_TIMEOUT	1000

struct cpld_spi_slave {
	struct spi_slave slave;
	unsigned int	max_hz;
	unsigned int	mode;
};
#define to_cpld_spi_slave(s) container_of(s, struct cpld_spi_slave, slave)

static spi8xxx_t *get_spi_bus(unsigned int bus)
{
    volatile spi8xxx_t *spi;

    if (bus == 0) {
        spi = (spi8xxx_t *) (MBCPLD_BASE + CPLD_SPI_BUS_0_OFFSET);
    } else if (bus == 1) {
        spi = (spi8xxx_t *) (MBCPLD_BASE + CPLD_SPI_BUS_1_OFFSET);
    } else if (bus == 2) {
        spi = (spi8xxx_t *) (MBCPLD_BASE + CPLD_SPI_BUS_2_OFFSET);
    } else if (bus == 3) {
        spi = (spi8xxx_t *) (MBCPLD_BASE + CPLD_SPI_BUS_3_OFFSET);
    } else if (bus == 4) {
        spi = (spi8xxx_t *) (MBCPLD_BASE + CPLD_SPI_BUS_4_OFFSET);
    } else if (bus == 5) {
        spi = (spi8xxx_t *) (MBCPLD_BASE + CPLD_SPI_BUS_5_OFFSET);
    } else if (bus == 6) {
        spi = (spi8xxx_t *) (MBCPLD_BASE + CPLD_SPI_BUS_6_OFFSET);
    } else {
        spi = (spi8xxx_t *) (UPCPLD_BASE + CPLD_SPI_BUS_7_OFFSET);
    }

    return spi;
}

int spi_cs_is_valid(unsigned int bus, unsigned int cs)
{
  int valid = 1;
  if (bus > NUM_BUSES) {
    valid = 0;
  } else if (cs > NUM_CHIPSELECTS) {
    valid = 0;
  }
  return valid;
}

/* Set SPI chip-select to '0' active. */
void spi_cs_activate(struct spi_slave *slave)
{
  volatile spi8xxx_t *spi;

  spi = get_spi_bus(slave->bus);

#ifdef CONFIG_OSM4F
  spi->cs = ((u16)slave->cs & 0x000F) | (spi->cs & 0xFFF0);
#endif

  spi->mode = (spi->mode & 0x7FFFFFFF);
}

/* Set SPI chip-select to '1' inactive. */
void spi_cs_deactivate(struct spi_slave *slave)
{
  volatile spi8xxx_t *spi;

  spi = get_spi_bus(slave->bus);

  spi->mode = (spi->mode | 0x80000000);
}

void spi_enable(struct spi_slave *slave)
{
  volatile spi8xxx_t *spi;

  spi = get_spi_bus(slave->bus);

  spi->mode = (spi->mode | 0x01000000);
}

void spi_disable(struct spi_slave *slave)
{
  volatile spi8xxx_t *spi;

  spi = get_spi_bus(slave->bus);

  spi->mode = (spi->mode & 0xFEFFFFFF);
}

struct spi_slave *spi_setup_slave(unsigned int bus, unsigned int cs,
		unsigned int max_hz, unsigned int mode)
{
	struct cpld_spi_slave *bss;

	if (!spi_cs_is_valid(bus, cs))
		return NULL;

	bss = malloc(sizeof(*bss));
	if (!bss)
		return NULL;

	bss->slave.bus = bus;
	bss->slave.cs = cs;
	bss->max_hz = max_hz;
	bss->mode = mode;

	debug("%s: bus:%i cs:%i freq:%i mode:%i\n", __func__,
		bus, cs, max_hz, mode);

	return &bss->slave;
}

void spi_free_slave(struct spi_slave *slave)
{
	struct cpld_spi_slave *bss = to_cpld_spi_slave(slave);
	free(bss);
}

void spi_init(void)
{
    volatile spi8xxx_t *spi;
    unsigned int tmpdin, bus;

    printf("\jun debug at here for spi init\n");

    for (bus = 0; bus <= NUM_BUSES; bus++) {
        spi = get_spi_bus(bus);
        /*
        * SPI pins on the MPC83xx are not muxed, so all we do is initialize
        * some registers
        */
        spi->mode = SPI_MODE_REV | SPI_MODE_MS | SPI_MODE_EN;
        spi->mode = (spi->mode & 0xfff0ffff) | (1 << 16); /* Use SYSCLK / 8
                                                            (16.67MHz typ.) */
        spi->event = 0xffffffff;	/* Clear all SPI events */
        spi->mask = 0x00000000;	/* Mask  all SPI interrupts */
        spi->com = 0;		/* LST bit doesn't do anything, so disregard */
        /* read rx reg to clear NE flag */
        tmpdin = spi->rx;
    }
}

int spi_claim_bus(struct spi_slave *slave)
{
  spi_cs_activate(slave);

  return 0;
}

void spi_release_bus(struct spi_slave *slave)
{

}

int spi_xfer(struct spi_slave *slave, unsigned int bitlen, const void *dout,
		void *din, unsigned long flags)
{
	volatile spi8xxx_t *spi;
	struct cpld_spi_slave *bss = to_cpld_spi_slave(slave);
	unsigned int tmpdout, tmpdin, event;
	char *doutPtr = dout;
	char *dinPtr  = din;
	int numBlks = bitlen / 32 + (bitlen % 32 ? (((bitlen % 32) > 16) ? 2 : 1) : 0);
	int tm, isRead = 0;
	unsigned char charSize = 32;
  double spi_clk_rate = (((double) CONFIG_SYS_CLK_FREQ) / 4.0);
  unsigned spi_mode;

  spi = get_spi_bus(slave->bus);

  spi_mode = spi->mode; /* read from cpld */
  printf("\jun debug at here for spi init\n");

	debug("spi_xfer: slave %u:%u dout %08X din %08X bitlen %u\n",
	      slave->bus, slave->cs, *(uint *) dout, *(uint *) din, bitlen);

        /* read rx reg to clear NE flag */
        tmpdin = spi->rx;

	if (flags & SPI_XFER_BEGIN)
		spi_cs_activate(slave);

        /* Set spi mode */
        spi_mode = bss->mode;

        /* Set spi clk divider */
        if (((double) bss->max_hz) < (spi_clk_rate / 16.0)) {
          spi_mode = (spi_mode | 0x08000000); /* DIV16 = 1 */
          spi_clk_rate /= 16.0;
        } else {
          spi_mode = (spi_mode & 0xf7ffffff); /* DIV16 = 0 */
        }
        if (((double) bss->max_hz) < (spi_clk_rate / 2.0)) {
          spi_mode = (spi_mode | 0x00080000); /* DIV16 = 1 */
          spi_clk_rate /= 2.0;
        } else {
          spi_mode = (spi_mode & 0xfff7ffff); /* DIV16 = 0 */
        }
        if (((double) bss->max_hz) < (spi_clk_rate / 2.0)) {
          spi_mode = (spi_mode | 0x00040000); /* DIV8 = 0 */
          spi_clk_rate /= 2.0;
        } else {
          spi_mode = (spi_mode & 0xfffbffff); /* DIV8 = 0 */
        }
        if (((double) bss->max_hz) < (spi_clk_rate / 2.0)) {
          spi_mode = (spi_mode | 0x00020000); /* DIV4 = 1 */
          spi_clk_rate /= 2.0;
        } else {
          spi_mode = (spi_mode & 0xfffdffff); /* DIV4 = 0 */
        }
        if (((double) bss->max_hz) < (spi_clk_rate / 2.0)) {
          spi_mode = (spi_mode | 0x00010000); /* DIV2 = 1 */
        } else {
          spi_mode = (spi_mode & 0xfffeffff); /* DIV2 = 0 */
        }

        /* write to cpld */
        spi->mode = spi_mode | SPI_MODE_EN;
	spi->event = 0xffffffff;	/* Clear all SPI events */

	/* handle data in 32-bit chunks */
	while (numBlks--) {
		tmpdout = 0;
		charSize = ((bitlen >= 32) ? 32 : (bitlen >= 16) ? 16 : bitlen);

		/* Shift data so it's msb-justified */
		tmpdout = *(u32 *) doutPtr >> (32 - charSize);

		/* The LEN field of the SPMODE register is set as follows:
		 *
		 * Bit length             setting
		 * len <= 4               3
		 * 4 < len <= 16          len - 1
		 * len > 16               0
		 */

		if (charSize <= 16) {
			if (charSize <= 4)
				spi->mode = (spi->mode & 0xff0fffff) | (3 << 20);
			else
				spi->mode = (spi->mode & 0xff0fffff) | ((charSize - 1) << 20);
			if (charSize == 16) {
				// if sending 16 bits, might have been between 17 and 31 left,
				// so we have one more go-around. Decrement by 16 bits.
				bitlen -= 16;
				doutPtr += 2;
			}
		} else {
			spi->mode = (spi->mode & 0xff0fffff);
			/* Set up the next iteration if sending > 32 bits */
			bitlen -= 32;
			doutPtr += 4;
		}

		spi->tx = tmpdout;	/* Write the data out */
		debug("*** spi_xfer: ... %08x written\n", tmpdout);

		/*
		 * Wait for SPI transmit to get out
		 * or time out (1 second = 1000 ms)
		 * The NE event must be read and cleared first
		 */
		for (tm = 0, isRead = 0; tm < SPI_TIMEOUT; ++tm) {
			event = spi->event;
			if (event & SPI_EV_NE) {
				tmpdin = spi->rx;
				spi->event |= SPI_EV_NE;
				isRead = 1;

				*(u32 *) dinPtr = (tmpdin << (32 - charSize));
				if (charSize == 16) {
					// Advance output buffer by 16 bits
					dinPtr += 2;
				}
				if (charSize == 32) {
					/* Advance output buffer by 32 bits */
					dinPtr += 4;
				}
			}
			/*
			 * Only bail when we've had both NE and NF events.
			 * This will cause timeouts on RO devices, so maybe
			 * in the future put an arbitrary delay after writing
			 * the device.  Arbitrary delays suck, though...
			 */
			if (isRead && (event & SPI_EV_NF))
				break;
		}
		if (tm >= SPI_TIMEOUT)
			puts("*** spi_xfer: Time out during SPI transfer\n");

		debug("*** spi_xfer: transfer ended. Value=%08x\n", tmpdin);
	}

	if (flags & SPI_XFER_END)
		spi_cs_deactivate(slave);

	return 0;
}
