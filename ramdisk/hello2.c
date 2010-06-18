#include <lib/types.h>
#include <lib/io.h>

void main()
{

    *((int*)0xb8000)=0x07690748;

}
