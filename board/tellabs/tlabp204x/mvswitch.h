
unsigned short mvswitch_read (unsigned char devad, unsigned char regad);
void mvswitch_write (unsigned char devad, unsigned char regad, unsigned short wrdata);

unsigned short mvswitch_poll (unsigned char devad, unsigned char regad, unsigned short rdmask,
			      unsigned short expValue, unsigned int loopmax,
			      int dly_us);

void mvswitch_init ( void );

int do_mvswitch (cmd_tbl_t *cmdtp, int flag, int argc, char *argv[]);

