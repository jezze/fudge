#include <arch.h>
#include <fudge.h>

void main()
{

    char c;

    while (!call_read(CALL_DI, 0, 1, &c));

}

