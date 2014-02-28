enum call_index
{

    CALL_DR                             = 0x01,
    CALL_DW                             = 0x02,
    CALL_I0                             = 0x03,
    CALL_O0                             = 0x04,
    CALL_I1                             = 0x05,
    CALL_O1                             = 0x06,
    CALL_L0                             = 0x07,
    CALL_L1                             = 0x08,
    CALL_L2                             = 0x09,
    CALL_L3                             = 0x0A,
    CALL_L4                             = 0x0B,
    CALL_L5                             = 0x0C,
    CALL_L6                             = 0x0D,
    CALL_L7                             = 0x0E,
    CALL_L8                             = 0x0F,
    CALL_P0                             = 0x10,
    CALL_P1                             = 0x11,
    CALL_P2                             = 0x12,
    CALL_P3                             = 0x13,
    CALL_P4                             = 0x14,
    CALL_P5                             = 0x15,
    CALL_P6                             = 0x16,
    CALL_P7                             = 0x17,
    CALL_P8                             = 0x18,
    CALL_E0                             = 0x19,
    CALL_E1                             = 0x1A,
    CALL_E2                             = 0x1B,
    CALL_E3                             = 0x1C,
    CALL_E4                             = 0x1D,
    CALL_E5                             = 0x1E,
    CALL_E6                             = 0x1F

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
