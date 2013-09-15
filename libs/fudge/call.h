enum call_index
{

    CALL_DR                             = 0x01,
    CALL_DW                             = 0x02,
    CALL_DI                             = 0x03,
    CALL_DO                             = 0x04,
    CALL_DC                             = 0x05,
    CALL_D0                             = 0x06,
    CALL_D1                             = 0x07,
    CALL_D2                             = 0x08,
    CALL_D3                             = 0x09,
    CALL_D4                             = 0x0A,
    CALL_D5                             = 0x0B,
    CALL_D6                             = 0x0C,
    CALL_D7                             = 0x0D,
    CALL_D8                             = 0x0E,
    CALL_D9                             = 0x0F,
    CALL_P0                             = 0x10,
    CALL_P1                             = 0x11,
    CALL_P2                             = 0x12,
    CALL_P3                             = 0x13,
    CALL_P4                             = 0x14,
    CALL_P5                             = 0x15,
    CALL_P6                             = 0x16,
    CALL_P7                             = 0x17,
    CALL_P8                             = 0x18

};

unsigned int call_bind(unsigned int index, unsigned int pindex, unsigned int cindex);
unsigned int call_close(unsigned int index);
unsigned int call_execute(unsigned int index);
unsigned int call_exit();
unsigned int call_open(unsigned int index, unsigned int pindex, unsigned int count, const char *path);
unsigned int call_load(unsigned int index);
unsigned int call_mount(unsigned int index, unsigned int pindex, unsigned int cindex);
unsigned int call_read(unsigned int index, unsigned int offset, unsigned int count, void *buffer);
unsigned int call_spawn(unsigned int index);
unsigned int call_unload(unsigned int index);
unsigned int call_write(unsigned int index, unsigned int offset, unsigned int count, void *buffer);
