enum call_index
{

    CALL_DI                             = 0x01,
    CALL_DO                             = 0x02,
    CALL_DC                             = 0x03,
    CALL_D0                             = 0x04,
    CALL_D1                             = 0x05,
    CALL_D2                             = 0x06,
    CALL_D3                             = 0x07,
    CALL_D4                             = 0x08,
    CALL_D5                             = 0x09,
    CALL_D6                             = 0x0A,
    CALL_D7                             = 0x0B,
    CALL_D8                             = 0x0C,
    CALL_D9                             = 0x0D,
    CALL_DW                             = 0x0E,
    CALL_DR                             = 0x0F

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
