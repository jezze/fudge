#include <fudge.h>
#include <abi.h>
#include "port.h"
#include "sdl.h"
#include "glue.h"

#define HEAPSIZE                        0x01000000
#define HEAPGROW                        0x00100000
#define FILES                           16
#define PATHSIZE                        256
#define DIRECTORIES                     32

struct block
{

    unsigned int size;
    unsigned int free;

};

struct directory
{

    char path[PATHSIZE];
    unsigned int length;
    unsigned int target;
    unsigned int count;
    struct record *records;

};

struct port_file
{

    unsigned char *data;
    unsigned int size;
    unsigned int offset;
    unsigned int used;

};

static unsigned int memtarget;
static unsigned int memid;
static unsigned char *heap;
static unsigned int heaptop;
static unsigned int heapsize;
static unsigned int rover;
static struct port_file files[FILES];
static char root[PATHSIZE];
static struct directory directories[DIRECTORIES];
static unsigned int ndirectories;

void *memcpy(void *out, const void *in, size_t count)
{

    unsigned char *o = out;
    const unsigned char *i = in;
    size_t k;

    for (k = 0; k < count; k++)
        o[k] = i[k];

    return out;

}

void *memmove(void *out, const void *in, size_t count)
{

    unsigned char *o = out;
    const unsigned char *i = in;
    size_t k;

    if (o < i)
    {

        for (k = 0; k < count; k++)
            o[k] = i[k];

    }

    else
    {

        for (k = count; k > 0; k--)
            o[k - 1] = i[k - 1];

    }

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

int memcmp(const void *in1, const void *in2, size_t count)
{

    const unsigned char *i1 = in1;
    const unsigned char *i2 = in2;
    size_t k;

    for (k = 0; k < count; k++)
    {

        if (i1[k] != i2[k])
            return i1[k] - i2[k];

    }

    return 0;

}

size_t strlen(const char *in)
{

    size_t k = 0;

    while (in[k])
        k++;

    return k;

}

char *strcpy(char *out, const char *in)
{

    return memcpy(out, in, strlen(in) + 1);

}

char *strncpy(char *out, const char *in, size_t count)
{

    size_t k;

    for (k = 0; k < count && in[k]; k++)
        out[k] = in[k];

    for (; k < count; k++)
        out[k] = '\0';

    return out;

}

int strncmp(const char *in1, const char *in2, size_t count)
{

    size_t k;

    for (k = 0; k < count; k++)
    {

        if (in1[k] != in2[k] || !in1[k])
            return (unsigned char)in1[k] - (unsigned char)in2[k];

    }

    return 0;

}

int strcmp(const char *in1, const char *in2)
{

    return strncmp(in1, in2, (size_t)-1);

}

static int lower(int c)
{

    return (c >= 'A' && c <= 'Z') ? c + 'a' - 'A' : c;

}

int strncasecmp(const char *in1, const char *in2, size_t count)
{

    size_t k;

    for (k = 0; k < count; k++)
    {

        int c1 = lower((unsigned char)in1[k]);
        int c2 = lower((unsigned char)in2[k]);

        if (c1 != c2 || !c1)
            return c1 - c2;

    }

    return 0;

}

int strcasecmp(const char *in1, const char *in2)
{

    return strncasecmp(in1, in2, (size_t)-1);

}

char *strchr(const char *in, int c)
{

    size_t k;

    for (k = 0; in[k]; k++)
    {

        if (in[k] == (char)c)
            return (char *)in + k;

    }

    return (c) ? 0 : (char *)in + k;

}

char *strrchr(const char *in, int c)
{

    char *last = 0;
    size_t k;

    for (k = 0; in[k]; k++)
    {

        if (in[k] == (char)c)
            last = (char *)in + k;

    }

    return (c) ? last : (char *)in + k;

}

int atoi(const char *in)
{

    int negative = (in[0] == '-');

    return (negative) ? -(int)cstring_read_value((char *)in + 1, cstring_length((char *)in + 1), 10) : (int)cstring_read_value((char *)in, cstring_length((char *)in), 10);

}

static struct block *getblock(unsigned int offset)
{

    return (struct block *)(heap + offset);

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

/* the rover must stay on a block boundary, so a merge that swallows it moves it back */
static void merge(struct block *block, unsigned int offset)
{

    while (offset + block->size < heaptop)
    {

        struct block *next = getblock(offset + block->size);

        if (!next->free)
            break;

        block->size += next->size;

    }

    if (rover > offset && rover < offset + block->size)
        rover = offset;

}

void *malloc(size_t size)
{

    unsigned int need = sizeof (struct block) + ((size + 7) & ~7);
    unsigned int offset = rover;
    unsigned int wrapped = 0;

    if (!heap)
        return 0;

    while (!wrapped || offset < rover)
    {

        struct block *block;

        if (offset >= heaptop)
        {

            if (wrapped)
                break;

            wrapped = 1;
            offset = 0;

            continue;

        }

        block = getblock(offset);

        if (block->free)
        {

            merge(block, offset);

            if (block->size >= need)
            {

                if (block->size - need >= sizeof (struct block) + 16)
                {

                    struct block *rest = getblock(offset + need);

                    rest->size = block->size - need;
                    rest->free = 1;
                    block->size = need;

                }

                block->free = 0;
                rover = offset;

                return block + 1;

            }

        }

        offset += block->size;

    }

    if (!grow(heaptop + need))
        return 0;

    getblock(heaptop)->size = need;
    getblock(heaptop)->free = 0;
    heaptop += need;

    return getblock(heaptop - need) + 1;

}

void *calloc(size_t count, size_t size)
{

    void *pointer = malloc(count * size);

    if (pointer)
        memset(pointer, 0, count * size);

    return pointer;

}

void free(void *pointer)
{

    struct block *block;
    unsigned int offset;

    if (!pointer)
        return;

    block = (struct block *)pointer - 1;
    offset = (unsigned char *)block - heap;
    block->free = 1;

    merge(block, offset);

    if (offset + block->size >= heaptop)
    {

        heaptop = offset;

        if (rover >= heaptop)
            rover = 0;

    }

}

void *realloc(void *pointer, size_t size)
{

    void *out;
    unsigned int old;

    if (!pointer)
        return malloc(size);

    old = ((struct block *)pointer - 1)->size - sizeof (struct block);

    if (old >= size)
        return pointer;

    out = malloc(size);

    if (out)
    {

        memcpy(out, pointer, old);
        free(pointer);

    }

    return out;

}

/* paths are relative to the data directory: "data/X" and "X" name the same file */
static unsigned int resolve(char *out, const char *path)
{

    unsigned int offset = 0;

    if (!strncmp(path, "./", 2))
        path += 2;

    if (!strncasecmp(path, "data/", 5))
        path += 5;

    offset += cstring_write(out, PATHSIZE, root, offset);
    offset += cstring_write(out, PATHSIZE, (char *)path, offset);
    offset += cstring_write_zero(out, PATHSIZE, offset);

    return offset == strlen(root) + strlen(path) + 1;

}

/* a directory is listed once and kept, so finding a file, or that it is missing, takes no requests */
static struct directory *getdirectory(char *path, unsigned int length)
{

    struct directory *directory;
    unsigned char data[MESSAGE_SIZE];
    unsigned int offset = 0;
    unsigned int count;
    unsigned int id;
    unsigned int i;

    for (i = 0; i < ndirectories; i++)
    {

        directory = &directories[i];

        if (directory->length == length && !strncmp(directory->path, path, length))
            return directory;

    }

    if (ndirectories == DIRECTORIES || length >= PATHSIZE)
        return 0;

    directory = &directories[ndirectories++];
    directory->length = length;
    directory->count = 0;
    directory->records = 0;

    memcpy(directory->path, path, length);

    directory->path[length] = '\0';
    directory->target = fs_auth(directory->path);
    id = (directory->target) ? fs_walk(1, directory->target, 0, directory->path) : 0;

    while (id && (count = fs_read(1, directory->target, id, data, MESSAGE_SIZE, offset)))
    {

        struct record *records = realloc(directory->records, (directory->count + count / sizeof (struct record)) * sizeof (struct record));

        if (!records)
            break;

        directory->records = records;

        for (i = 0; i < count; i += sizeof (struct record))
        {

            struct record *record = (struct record *)(data + i);

            directory->records[directory->count++] = record[0];
            offset = record->offset;

        }

    }

    return directory;

}

static unsigned int lookup(const char *path, unsigned int *target, struct record *record)
{

    char full[PATHSIZE];
    struct directory *directory;
    char *base;
    unsigned int length;
    unsigned int i;

    if (!resolve(full, path))
        return 0;

    base = strrchr(full, '/');

    if (!base)
        return 0;

    directory = getdirectory(full, base - full);
    base++;
    length = strlen(base);

    if (!directory)
        return 0;

    for (i = 0; i < directory->count; i++)
    {

        struct record *current = &directory->records[i];

        if (current->length == length && !strncmp(current->name, base, length))
        {

            target[0] = directory->target;
            record[0] = current[0];

            /* some services (ext2srv) list files without their size */
            if (record->type == RECORD_TYPE_NORMAL && !record->size && !fs_stat(1, target[0], current->id, record))
                return 0;

            return current->id;

        }

    }

    return 0;

}

int port_exists(const char *path)
{

    struct record record;
    unsigned int target;

    return lookup(path, &target, &record) != 0;

}

int port_isdirectory(const char *path)
{

    struct record record;
    unsigned int target;

    return lookup(path, &target, &record) && record.type == RECORD_TYPE_DIRECTORY;

}

FILE *fopen(const char *path, const char *mode)
{

    struct record record;
    unsigned int target;
    unsigned int id;
    unsigned int i;

    if (mode[0] != 'r')
        return 0;

    id = lookup(path, &target, &record);

    if (!id || record.type != RECORD_TYPE_NORMAL)
    {

        return 0;

    }

    for (i = 0; i < FILES; i++)
    {

        struct port_file *file = &files[i];

        if (file->used)
            continue;

        file->data = malloc(record.size + 1);

        if (!file->data)
            return 0;

        file->size = fs_read_full(1, target, id, file->data, record.size, 0);

        file->offset = 0;
        file->used = 1;

        return file;

    }

    return 0;

}

size_t fread(void *out, size_t size, size_t count, FILE *file)
{

    size_t total = size * count;

    if (!size || file->offset + total > file->size)
    {

        count = (size) ? (file->size - file->offset) / size : 0;
        total = size * count;

    }

    memcpy(out, file->data + file->offset, total);

    file->offset += total;

    return count;

}

/* nothing is ever opened for writing */
size_t fwrite(const void *in, size_t size, size_t count, FILE *file)
{

    return 0;

}

int fseek(FILE *file, long offset, int whence)
{

    long base = (whence == SEEK_END) ? (long)file->size : (whence == SEEK_CUR) ? (long)file->offset : 0;

    if (base + offset < 0 || base + offset > (long)file->size)
        return -1;

    file->offset = base + offset;

    return 0;

}

int port_filesize(FILE *file)
{

    return file->size;

}

long ftell(FILE *file)
{

    return file->offset;

}

int fclose(FILE *file)
{

    free(file->data);

    file->used = 0;

    return 0;

}

int remove(const char *path)
{

    return -1;

}

unsigned int time(void *unused)
{

    return SDL_GetTicks();

}

void longjmp(jmp_buf buffer, int value)
{

    __builtin_longjmp(buffer, 1);

}

void exit(int status)
{

    glue_exit(status);

}

/* a mem: file of our own holds the heap, named pop and the first free number */
unsigned int port_init(char *path)
{

    unsigned int memparent;
    unsigned int i;

    cstring_write_zero(root, PATHSIZE, cstring_write(root, PATHSIZE, path, 0));

    memtarget = fs_auth("mem:");
    memparent = (memtarget) ? fs_walk(1, memtarget, 0, "mem:") : 0;

    for (i = 0; i < 64 && memparent && !memid; i++)
    {

        char name[32];

        cstring_write_fmt(name, 32, 0, "pop%u\\0", &i);

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

