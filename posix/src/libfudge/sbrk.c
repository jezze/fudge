#include <_syslist.h>

void *sbrk(int incr)
{ 

    extern char end;
    static char *heap_end; 
    char *prev_heap_end; 

    if (heap_end == 0)
        heap_end = & end; 

    prev_heap_end = heap_end; 
    heap_end += incr; 

    return (void *)prev_heap_end;

} 
