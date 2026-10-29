#include <fudge.h>
#include <abi.h>
#include "port.h"
#include "glue.h"

#define HEAPSIZE                        0x02000000
#define HEAPGROW                        0x00100000

static unsigned int memtarget;
static unsigned int memid;
static unsigned char *heap;
static unsigned int heaptop;
static unsigned int heapsize;

void *memcpy(void *out, const void *in, size_t count)
{

    unsigned char *o = out;
    const unsigned char *i = in;
    size_t k;

    for (k = 0; k < count; k++)
        o[k] = i[k];

    return out;

}

void *memset(void *out, int value, size_t count)
{

    unsigned char *o = out;
    size_t k;

    for (k = 0; k < count; k++)
        o[k] = value;

    return out;

}

int strcmp(const char *in1, const char *in2)
{

    size_t k;

    for (k = 0; in1[k] == in2[k]; k++)
    {

        if (!in1[k])
            return 0;

    }

    return (unsigned char)in1[k] - (unsigned char)in2[k];

}

size_t strlen(const char *in)
{

    size_t k;

    for (k = 0; in[k]; k++);

    return k;

}

/* the mem: file refuses writes past its end, so writing its last byte grows it */
static unsigned int grow(unsigned int size)
{

    unsigned char zero = 0;

    if (size <= heapsize)
        return 1;

    size = (size + HEAPGROW - 1) & ~(HEAPGROW - 1);

    if (size > HEAPSIZE || fs_write(1, memtarget, memid, &zero, 1, size - 1) != 1)
        return 0;

    heapsize = size;

    return 1;

}

/* the emulator only allocates while it starts and frees nothing, the whole heap goes when it ends */
void *malloc(size_t size)
{

    unsigned int need = (size + 7) & ~7;
    void *pointer;

    if (!heap || need < size || !grow(heaptop + need))
        return 0;

    pointer = heap + heaptop;
    heaptop += need;

    return pointer;

}

/* a mem: file of our own holds the heap, named snes and the first free number */
unsigned int port_init(void)
{

    unsigned int memparent;
    unsigned int i;

    memtarget = fs_auth("mem:");
    memparent = (memtarget) ? fs_walk(1, memtarget, 0, "mem:") : 0;

    for (i = 0; i < 64 && memparent && !memid; i++)
    {

        char name[32];

        cstring_write_fmt(name, 32, 0, "snes%u\\0", &i);

        memid = fs_create(1, memtarget, memparent, name, cstring_length(name));

    }

    if (memid)
        heap = fs_mmap(1, memtarget, memid, HEAPSIZE);

    return heap != 0;

}

void port_destroy(void)
{

    if (heap)
        fs_munmap(1, memtarget, memid);

    if (memid)
        fs_remove(1, memtarget, fs_walk(1, memtarget, 0, "mem:"), memid);

}

