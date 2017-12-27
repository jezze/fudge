#include <abi.h>
#include <fudge.h>
#include <elf/elf.h>

void main(void)
{

    file_open(FILE_PI);
    call_unload(FILE_PI);
    file_close(FILE_PI);

}

