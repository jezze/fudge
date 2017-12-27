#include <abi.h>
#include <fudge.h>
#include <format/elf.h>

void main(void)
{

    file_open(FILE_PI);
    call_unload(FILE_PI);
    file_close(FILE_PI);

}

