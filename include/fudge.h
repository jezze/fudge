#include <fudge/memory.h>
#include <fudge/ascii.h>
#include <utf8/utf8.h>

#define FUDGE_BSIZE                     0x1000

enum
{

    CALL_DR                             = 0x01,
    CALL_DW                             = 0x02,
    CALL_DT                             = 0x03,
    CALL_I0                             = 0x04,
    CALL_O0                             = 0x05,
    CALL_I1                             = 0x06,
    CALL_O1                             = 0x07,
    CALL_I2                             = 0x08,
    CALL_O2                             = 0x09,
    CALL_I3                             = 0x0A,
    CALL_O3                             = 0x0B,
    CALL_I4                             = 0x0C,
    CALL_O4                             = 0x0D,
    CALL_I5                             = 0x0E,
    CALL_O5                             = 0x0F,
    CALL_I6                             = 0x10,
    CALL_O6                             = 0x11,
    CALL_I7                             = 0x12,
    CALL_O7                             = 0x13,
    CALL_I8                             = 0x14,
    CALL_O8                             = 0x15,
    CALL_I9                             = 0x16,
    CALL_O9                             = 0x17,
    CALL_L0                             = 0x18,
    CALL_L1                             = 0x19,
    CALL_L2                             = 0x1A,
    CALL_L3                             = 0x1B,
    CALL_L4                             = 0x1C,
    CALL_L5                             = 0x1D,
    CALL_L6                             = 0x1E,
    CALL_L7                             = 0x1F

};

unsigned int call_auth(unsigned int channel);
unsigned int call_bind(unsigned int mount, unsigned int pindex, unsigned int cindex);
unsigned int call_close(unsigned int index);
unsigned int call_execute(unsigned int index);
unsigned int call_exit();
unsigned int call_load(unsigned int index);
unsigned int call_mount(unsigned int channel, unsigned int mount, unsigned int index);
unsigned int call_open(unsigned int index);
unsigned int call_read(unsigned int index, unsigned int offset, unsigned int count, void *buffer);
unsigned int call_spawn(unsigned int index);
unsigned int call_unload(unsigned int index);
unsigned int call_walk(unsigned int index, unsigned int pindex, unsigned int count, const char *path);
unsigned int call_write(unsigned int index, unsigned int offset, unsigned int count, void *buffer);
