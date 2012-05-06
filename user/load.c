#include <fudge.h>

static char module[0x8000];
static unsigned int moduleCount;
static char symbol[0x1000];
static unsigned int symbolCount;

void main(int argc, char *argv[])
{

    unsigned int id = call_open("/ramdisk/boot/fudge.map");
    symbolCount = call_read(id, 0, 0x1000, symbol);
    call_close(id);

    moduleCount = call_read(FILE_STDIN, 0, 0x8000, module);

    call_load(FILE_STDIN);

}

