#include <common.h>
#include <command.h>
#include <config.h>
#include <asm/types.h>
#include <malloc.h>
#include <spi.h>
#include "cpld.h"

struct reg_cfg
{
    unsigned char reg;
    unsigned char value;
};

//Include configure sequence
struct reg_cfg regs_cfg_seq[] = {
    {0x7f, 0x00}, //switch to page 0
    {0x0d, 0x01},
    {0x0a, 0x00},
    {0x10, 0x1f},
    {0x11, 0x40},
    {0x12, 0x00},
    {0x13, 0x01},
    {0x18, 0x1f},
    {0x19, 0x40},
    {0x1a, 0x00},
    {0x1b, 0x01},
    {0x22, 0x01},
    {0x23, 0xf4},
    {0x28, 0x1f},
    {0x29, 0x40},
    {0x2a, 0x00},
    {0x2b, 0x01},
    {0x31, 0x72},
    {0x32, 0x77},
    {0x33, 0x83},
    {0x36, 0x72},
    {0x37, 0x77},
    {0x38, 0x83},
    {0x45, 0x08},
    {0x47, 0x14},
    {0x48, 0x14},
    {0x4a, 0x14},
    {0x58, 0x9c},
    {0x59, 0x40},
    {0x5a, 0x07},
    {0x5b, 0x98},
    {0x5c, 0x00},
    {0x5d, 0xff},
    {0x5e, 0x00},
    {0x5f, 0xe3},
    {0x7f, 0x01}, //switch to page1
    {0x00, 0x00}, //page1 registers start from 0x80
    {0x01, 0x00},
    {0x02, 0x08},
    {0x03, 0x00},
    {0x04, 0x00},
    {0x05, 0x08},
    {0x06, 0x02},
    {0x07, 0x5f},
    {0x08, 0x7f},
    {0x09, 0x02},
    {0x0a, 0x5f},
    {0x0b, 0x7f},
    {0x12, 0x02},
    {0x13, 0xaa},
    {0x14, 0x69},
    {0x15, 0x00},
    {0x16, 0x00},
    {0x17, 0xc8},
    {0x30, 0x1f},
    {0x31, 0x0f},
    {0x38, 0x55},
    {0x66, 0xb1},
    {0x67, 0xc1},
    {0x68, 0x80},
    {0x77, 0x05},
    {0x7f, 0x00},//switch to page 0
    {0x0d, 0x00}
};

static char zl30155_read(struct spi_slave *slave, unsigned char addr)
{
    unsigned char dout[2];
    unsigned char din[2];

    dout[0] = addr | 0x80; 
    dout[1] = 0;
    
    mbcpld_write(0xd8, 0x9);    
    spi_xfer(slave, 16, dout, din, 0);
    mbcpld_write(0xd8, 0xb);
    
    return din[1];
}

static void zl30155_write(struct spi_slave *slave, unsigned char addr, unsigned char data)
{
    unsigned char dout[2];
    unsigned char din[2];

    dout[0] = addr & 0x7f; 
    dout[1] = data;
    
    mbcpld_write(0xd8, 0x9);    
    spi_xfer(slave, 16, dout, din, 0);
    mbcpld_write(0xd8, 0xb);  
}

int zl30155_init(struct spi_slave *slave)
{
    unsigned int i, elments;
    unsigned char tmp;
    unsigned long time;
    char buf[1024] ={0};
    unsigned len = 0;

    //Deassert zl30155 reset
    tmp = mbcpld_read(0x42);
    mbcpld_write(0x42, tmp | 0x1);

    //Wait for chip ready(at least 30ms) 
    printf("Wait ZL30155 ready ");
    
    mdelay(30);
    
    for (i = 0; i < 30; i++)
    {    
        tmp = zl30155_read(slave, 0); 
         if (!(i % 10))
           printf(".", tmp);    
        if ((tmp & 0x9f) == 0x85)
        {
            break;
        }
        
        mdelay(1); 
    }

    if (i == 30)
    {
        printf("time out!\n");
        return 1;
    }
    else
    {
        printf("ok. Used %u ms.\n", 30 + i);
    }
    
    elments = sizeof(regs_cfg_seq)/sizeof(regs_cfg_seq[0]);
    for (i = 0; i < elments; i++)
    {
        zl30155_write(slave, regs_cfg_seq[i].reg, regs_cfg_seq[i].value);
    }

    //Wait for  zl30155 lock (1s)    
    printf("Wait ZL30155 dpll lock ");
    for (i = 0; i < 1500; i++)
    {        
        mbcpld_write(0xb, 0x60);
        if (!(i % 100))
            printf(".", tmp);
        tmp = mbcpld_read(0xb);
        if (!(tmp & 0x60))
        {
            break;
        }
        
        mdelay(1); 
    }
    
    if (i == 1500)
    { 
        printf(buf);
        printf("time out!\n"); 
        printf("Reg 0xb = %#x\n", tmp);
        return 1;
    }
    else
    {
        printf("ok. Used %u ms.\n", i);
    }
    //Deassert si5317 reset
    tmp = mbcpld_read(0x42);
    mbcpld_write(0x42, tmp | 0x6);

    //Wait for si5317 lock(1.2s)  
    printf("Wait SI5317 clock lock ");
    for (i = 0; i < 2000; i++)
    {               
        mbcpld_write(0xb, 0x06);
        tmp = mbcpld_read(0xb);
        if (!(i % 100))
            printf(".", tmp);
        if (!(tmp & 0x06))
        {            
            break;
        }
        
        mdelay(1);
    }

    if (i == 2000)
    {    
        printf(buf);
        printf("time out!\n");  
        printf("Reg 0xb = %#x\n", tmp);
        return 1;
    }
    else
    {
        printf("ok. Used %u ms.\n", i);
    }
    
    printf(buf);

    return 0;
}

int do_zl30155 (cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct spi_slave *slave;
    unsigned char addr;
    unsigned char data;
    unsigned long i;
    

	if (argc >= 2)
    {

    	slave = spi_setup_slave(0, 0, 1000000, 0x07000000);
        if (!slave) 
        {
    		printf("Setup slave failed!\n");
    		return 1;
    	}
        
        if ((!strcmp(argv[1], "init")) && (argc == 2)) 
        {
            i = 3;
            while(zl30155_init(slave) && (i-- > 0));
        }
        else if ((!strcmp(argv[1], "r")) && (argc >= 3)) 
        {
            addr = (unsigned char)simple_strtoul(argv[2], NULL, 16);
            i = 1;
 
            if (argc >= 4) 
            {
                i = simple_strtoul(argv[3], NULL, 10);
                if (i < 1) 
                { 
                    i = 1; 
                }
                
                if (i > 255) 
                { 
                    i = 255; 
                }
            }
            
            while (i > 0) 
            {
                data = zl30155_read(slave, addr);
                printf ("[0x%02x] : 0x%02x\n", ((unsigned int) addr), ((unsigned int) data));
                addr++;
                i--;
            }
        }
        else if ((!strcmp(argv[1], "w")) && (argc == 4)) 
        {
            addr = (unsigned char)simple_strtoul(argv[2], NULL, 16);
            data = (unsigned char)simple_strtoul(argv[3], NULL, 16);
            zl30155_write(slave, addr, data);
            printf ("[0x%02x] : 0x%02x\n", ((unsigned int) addr), ((unsigned int) data));
        } 
        else 
        {
            printf("usage -- zl30155 {r|w} <addr> {<data>|[<length>]}\n");
        }

        spi_release_bus(slave);
    	spi_free_slave(slave);

    } 
    else 
    {
        printf("usage -- zl30155 {r|w} <addr> {<data>|[<length>]}\n");
    }

	return 0;
}

/***************************************************/

U_BOOT_CMD(
	zl30155,	5,	1,	do_zl30155,
	"ZL30155 utility command",
	"zl30155 init\n"
	"zl30155 w <addr> <data>\n"
	"zl30155 r <addr> [<length>]\n"
);

