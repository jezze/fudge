#include <abi.h>
#include <fudge.h>
#include <format/elf.h>

void main(void)
{

    file_open(CALL_PI);
    call_unload(CALL_PI);
    file_close(CALL_PI);

}

