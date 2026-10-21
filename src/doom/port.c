#include <fudge.h>
#include <abi.h>
#include "port.h"

#define HEAPSIZE                        0x02000000
#define HEAPGROW                        0x00100000
#define FILES                           16
#define DESCRIPTORS                     8
#define PATHSIZE                        256
#define ERRORSIZE                       512
#define NUMBERSIZE                      32
#define STANDARDS                       2

struct block
{

    unsigned int size;
    unsigned int free;

};

struct port_file
{

    unsigned char *data;
    unsigned int size;
    unsigned int offset;
    unsigned int used;

};

struct descriptor
{

    unsigned int target;
    unsigned int id;
    unsigned int size;
    unsigned int offset;
    unsigned int used;

};

struct location
{

    unsigned int target;
    unsigned int id;
    unsigned int type;
    unsigned int size;

};

struct output
{

    char *out;
    size_t count;
    size_t offset;

};

struct spec
{

    unsigned int left;
    unsigned int zero;
    unsigned int plus;
    unsigned int space;
    unsigned int alternate;
    int width;
    int precision;

};

struct division
{

    unsigned long long quotient;
    unsigned long long remainder;

};

int errno;
static struct port_file files[FILES];
static struct port_file standard[STANDARDS];
static struct descriptor descriptors[DESCRIPTORS];
FILE *port_stdout = &standard[0];
FILE *port_stderr = &standard[1];
static unsigned int memtarget;
static unsigned int memid;
static unsigned char *heap;
static unsigned int heaptop;
static unsigned int heapsize;
static unsigned int rover;
static char root[PATHSIZE];
static char lasterror[ERRORSIZE];
static char *tokenizer;
static unsigned int seed = 1;
static struct tm emptytime;

void *memcpy(void *out, const void *in, size_t count)
{

    buffer_copy(out, (void *)in, count);

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

void *memchr(const void *in, int c, size_t count)
{

    const unsigned char *i = in;
    size_t k;

    for (k = 0; k < count; k++)
    {

        if (i[k] == (unsigned char)c)
            return (void *)(i + k);

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

char *strcat(char *out, const char *in)
{

    strcpy(out + strlen(out), in);

    return out;

}

char *strncat(char *out, const char *in, size_t count)
{

    size_t length = strlen(out);
    size_t k;

    for (k = 0; k < count && in[k]; k++)
        out[length + k] = in[k];

    out[length + k] = '\0';

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

int strncasecmp(const char *in1, const char *in2, size_t count)
{

    size_t k;

    for (k = 0; k < count; k++)
    {

        int c1 = tolower((unsigned char)in1[k]);
        int c2 = tolower((unsigned char)in2[k]);

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

char *strstr(const char *in, const char *needle)
{

    size_t length = strlen(needle);
    size_t k;

    for (k = 0; in[k]; k++)
    {

        if (!strncmp(in + k, needle, length))
            return (char *)in + k;

    }

    return (length) ? 0 : (char *)in;

}

char *strdup(const char *in)
{

    char *out = malloc(strlen(in) + 1);

    return (out) ? strcpy(out, in) : 0;

}

char *strtok(char *in, const char *delimiters)
{

    char *start;

    if (in)
        tokenizer = in;

    if (!tokenizer)
        return 0;

    while (tokenizer[0] && strchr(delimiters, tokenizer[0]))
        tokenizer++;

    if (!tokenizer[0])
    {

        tokenizer = 0;

        return 0;

    }

    start = tokenizer;

    while (tokenizer[0] && !strchr(delimiters, tokenizer[0]))
        tokenizer++;

    if (tokenizer[0])
    {

        tokenizer[0] = '\0';
        tokenizer++;

    }

    else
    {

        tokenizer = 0;

    }

    return start;

}

char *strerror(int error)
{

    return "error";

}

int toupper(int c)
{

    return (c >= 'a' && c <= 'z') ? c - 'a' + 'A' : c;

}

int tolower(int c)
{

    return (c >= 'A' && c <= 'Z') ? c - 'A' + 'a' : c;

}

int isspace(int c)
{

    return c == ' ' || (c >= '\t' && c <= '\r');

}

int isdigit(int c)
{

    return c >= '0' && c <= '9';

}

int isxdigit(int c)
{

    return isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');

}

int isupper(int c)
{

    return c >= 'A' && c <= 'Z';

}

int islower(int c)
{

    return c >= 'a' && c <= 'z';

}

int isalpha(int c)
{

    return isupper(c) || islower(c);

}

int isalnum(int c)
{

    return isalpha(c) || isdigit(c);

}

int isprint(int c)
{

    return c >= ' ' && c < 0x7F;

}

int ispunct(int c)
{

    return isprint(c) && !isalnum(c) && c != ' ';

}

static int digitvalue(int c)
{

    if (isdigit(c))
        return c - '0';

    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;

    return 36;

}

unsigned long strtoul(const char *in, char **end, int base)
{

    unsigned long value = 0;
    unsigned int negative = 0;
    const char *start = in;
    const char *p = in;

    while (isspace((unsigned char)p[0]))
        p++;

    if (p[0] == '-' || p[0] == '+')
    {

        negative = (p[0] == '-');
        p++;

    }

    if ((base == 0 || base == 16) && p[0] == '0' && (p[1] == 'x' || p[1] == 'X') && isxdigit((unsigned char)p[2]))
    {

        base = 16;
        p += 2;

    }

    else if (base == 0)
    {

        base = (p[0] == '0') ? 8 : 10;

    }

    start = p;

    while (digitvalue((unsigned char)p[0]) < base)
    {

        value = value * base + digitvalue((unsigned char)p[0]);
        p++;

    }

    if (end)
        end[0] = (char *)((p == start) ? in : p);

    return (negative) ? -value : value;

}

long strtol(const char *in, char **end, int base)
{

    return (long)strtoul(in, end, base);

}

double strtod(const char *in, char **end)
{

    const char *p = in;
    double value = 0;
    double scale = 1;
    unsigned int negative = 0;
    unsigned int digits = 0;

    while (isspace((unsigned char)p[0]))
        p++;

    if (p[0] == '-' || p[0] == '+')
    {

        negative = (p[0] == '-');
        p++;

    }

    for (; isdigit((unsigned char)p[0]); p++, digits++)
        value = value * 10 + (p[0] - '0');

    if (p[0] == '.')
    {

        for (p++; isdigit((unsigned char)p[0]); p++, digits++)
        {

            scale /= 10;
            value += (p[0] - '0') * scale;

        }

    }

    if (digits && (p[0] == 'e' || p[0] == 'E'))
    {

        char *after;
        long exponent = strtol(p + 1, &after, 10);

        if (after != p + 1)
        {

            p = after;

            for (; exponent > 0; exponent--)
                value *= 10;

            for (; exponent < 0; exponent++)
                value /= 10;

        }

    }

    if (end)
        end[0] = (char *)((digits) ? p : in);

    return (negative) ? -value : value;

}

int atoi(const char *in)
{

    return (int)strtol(in, 0, 10);

}

long atol(const char *in)
{

    return strtol(in, 0, 10);

}

double atof(const char *in)
{

    return strtod(in, 0);

}

int abs(int value)
{

    return (value < 0) ? -value : value;

}

long labs(long value)
{

    return (value < 0) ? -value : value;

}

int rand(void)
{

    seed = seed * 1103515245 + 12345;

    return (seed >> 16) & RAND_MAX;

}

void srand(unsigned int value)
{

    seed = value;

}

static void swap(unsigned char *a, unsigned char *b, size_t size)
{

    size_t k;

    for (k = 0; k < size; k++)
    {

        unsigned char c = a[k];

        a[k] = b[k];
        b[k] = c;

    }

}

static void sift(unsigned char *base, size_t start, size_t count, size_t size, int (*compare)(const void *, const void *))
{

    size_t root = start;

    while (root * 2 + 1 < count)
    {

        size_t child = root * 2 + 1;

        if (child + 1 < count && compare(base + child * size, base + (child + 1) * size) < 0)
            child++;

        if (compare(base + root * size, base + child * size) >= 0)
            return;

        swap(base + root * size, base + child * size, size);

        root = child;

    }

}

void qsort(void *base, size_t count, size_t size, int (*compare)(const void *, const void *))
{

    unsigned char *b = base;
    size_t k;

    for (k = count / 2; k > 0; k--)
        sift(b, k - 1, count, size, compare);

    for (k = count; k > 1; k--)
    {

        swap(b, b + (k - 1) * size, size);
        sift(b, 0, k - 1, size, compare);

    }

}

char *getenv(const char *name)
{

    return 0;

}

int system(const char *command)
{

    return -1;

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

/* a path without a service is relative to the data directory */
static unsigned int resolve(char *out, const char *path)
{

    unsigned int offset = 0;

    if (!strncmp(path, "./", 2))
        path += 2;

    if (!strchr(path, ':'))
        offset += cstring_write(out, PATHSIZE, root, offset);

    offset += cstring_write(out, PATHSIZE, (char *)path, offset);
    offset += cstring_write_zero(out, PATHSIZE, offset);

    return offset < PATHSIZE;

}

static struct location locate(const char *path)
{

    struct location location;
    struct record record;
    char full[PATHSIZE];

    buffer_clear(&location, sizeof (struct location));

    if (!resolve(full, path))
        return location;

    location.target = fs_auth(full);
    location.id = (location.target) ? fs_walk(1, location.target, 0, full) : 0;

    if (location.id && fs_stat(1, location.target, location.id, &record))
    {

        location.type = record.type;
        location.size = record.size;

    }

    else
    {

        location.id = 0;

    }

    return location;

}

int access(const char *path, int mode)
{

    if (mode & W_OK)
        return -1;

    return (locate(path).id) ? 0 : -1;

}

int stat(const char *path, struct stat *out)
{

    struct location location = locate(path);

    if (!location.id)
        return -1;

    out->st_mode = (location.type == RECORD_TYPE_DIRECTORY) ? 0x4000 : 0x8000;
    out->st_size = location.size;

    return 0;

}

/* nothing is written, so files are only read, all at once */
FILE *fopen(const char *path, const char *mode)
{

    struct location location;
    unsigned int i;

    if (mode[0] != 'r')
        return 0;

    location = locate(path);

    if (!location.id || location.type != RECORD_TYPE_NORMAL)
        return 0;

    for (i = 0; i < FILES; i++)
    {

        struct port_file *file = &files[i];

        if (file->used)
            continue;

        file->data = malloc(location.size + 1);

        if (!file->data)
            return 0;

        file->size = fs_read_full(1, location.target, location.id, file->data, location.size, 0);
        file->offset = 0;
        file->used = 1;

        return file;

    }

    return 0;

}

int fclose(FILE *file)
{

    free(file->data);

    file->used = 0;

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

long ftell(FILE *file)
{

    return file->offset;

}

int fgetc(FILE *file)
{

    if (file->offset >= file->size)
        return EOF;

    return file->data[file->offset++];

}

char *fgets(char *out, int count, FILE *file)
{

    int k = 0;

    if (file->offset >= file->size || count < 1)
        return 0;

    while (k < count - 1 && file->offset < file->size)
    {

        char c = file->data[file->offset++];

        out[k++] = c;

        if (c == '\n')
            break;

    }

    out[k] = '\0';

    return out;

}

int feof(FILE *file)
{

    return file->offset >= file->size;

}

int fflush(FILE *file)
{

    return 0;

}

void setbuf(FILE *file, char *buffer)
{

}

int remove(const char *path)
{

    return -1;

}

int unlink(const char *path)
{

    return -1;

}

int rename(const char *from, const char *to)
{

    return -1;

}

int isatty(int fd)
{

    return 0;

}

char *mktemp(char *template)
{

    return template;

}

/* descriptors read straight from the service, so a wad is never held whole */
int open(const char *path, int flags, ...)
{

    struct location location;
    int i;

    if (flags & (O_WRONLY | O_RDWR | O_CREAT | O_TRUNC))
        return -1;

    location = locate(path);

    if (!location.id || location.type != RECORD_TYPE_NORMAL)
        return -1;

    for (i = 0; i < DESCRIPTORS; i++)
    {

        struct descriptor *descriptor = &descriptors[i];

        if (descriptor->used)
            continue;

        descriptor->target = location.target;
        descriptor->id = location.id;
        descriptor->size = location.size;
        descriptor->offset = 0;
        descriptor->used = 1;

        return i;

    }

    return -1;

}

int close(int fd)
{

    if (fd < 0 || fd >= DESCRIPTORS)
        return -1;

    descriptors[fd].used = 0;

    return 0;

}

int read(int fd, void *out, size_t count)
{

    struct descriptor *descriptor;
    unsigned int total;

    if (fd < 0 || fd >= DESCRIPTORS || !descriptors[fd].used)
        return -1;

    descriptor = &descriptors[fd];

    if (descriptor->offset + count > descriptor->size)
        count = descriptor->size - descriptor->offset;

    total = fs_read_full(1, descriptor->target, descriptor->id, out, count, descriptor->offset);
    descriptor->offset += total;

    return total;

}

int write(int fd, const void *in, size_t count)
{

    return -1;

}

off_t lseek(int fd, off_t offset, int whence)
{

    struct descriptor *descriptor;
    long base;

    if (fd < 0 || fd >= DESCRIPTORS || !descriptors[fd].used)
        return -1;

    descriptor = &descriptors[fd];
    base = (whence == SEEK_END) ? (long)descriptor->size : (whence == SEEK_CUR) ? (long)descriptor->offset : 0;

    if (base + offset < 0)
        return -1;

    descriptor->offset = base + offset;

    return descriptor->offset;

}

static struct division divide(unsigned long long n, unsigned long long d)
{

    struct division division;
    unsigned int i;

    division.quotient = 0;
    division.remainder = 0;

#ifdef __i386__
    /* when the quotient fits in 32 bits one divl does it, which is what the game's fixed point divisions mostly are */
    if (!(d >> 32) && (n >> 32) < d)
    {

        unsigned int quotient;
        unsigned int remainder;

        __asm__ ("divl %4" : "=a" (quotient), "=d" (remainder) : "0" ((unsigned int)n), "1" ((unsigned int)(n >> 32)), "rm" ((unsigned int)d));

        division.quotient = quotient;
        division.remainder = remainder;

        return division;

    }
#endif

    for (i = 64; i > 0; i--)
    {

        division.remainder = (division.remainder << 1) | ((n >> (i - 1)) & 1);

        if (division.remainder >= d)
        {

            division.remainder -= d;
            division.quotient |= 1ULL << (i - 1);

        }

    }

    return division;

}

static void put(struct output *output, char c)
{

    if (output->offset + 1 < output->count)
        output->out[output->offset] = c;

    output->offset++;

}

static void pad(struct output *output, char c, int count)
{

    for (; count > 0; count--)
        put(output, c);

}

/* the digits of a value, last first; a precision of zero prints nothing for zero */
static unsigned int digits(char *out, unsigned long long value, unsigned int base, unsigned int upper, int precision)
{

    const char *set = (upper) ? "0123456789ABCDEF" : "0123456789abcdef";
    unsigned int count = 0;

    while (value || (!count && precision))
    {

        struct division division = divide(value, base);

        out[count++] = set[division.remainder];
        value = division.quotient;

    }

    while ((int)count < precision && count < NUMBERSIZE)
        out[count++] = '0';

    return count;

}

static void putdigits(struct output *output, struct spec *spec, char *prefix, char *number, unsigned int count)
{

    int length = strlen(prefix) + count;
    unsigned int i;

    if (!spec->left && !spec->zero)
        pad(output, ' ', spec->width - length);

    for (i = 0; prefix[i]; i++)
        put(output, prefix[i]);

    if (!spec->left && spec->zero)
        pad(output, '0', spec->width - length);

    for (i = count; i > 0; i--)
        put(output, number[i - 1]);

    if (spec->left)
        pad(output, ' ', spec->width - length);

}

static char *sign(struct spec *spec, unsigned int negative)
{

    if (negative)
        return "-";

    if (spec->plus)
        return "+";

    if (spec->space)
        return " ";

    return "";

}

static void putinteger(struct output *output, struct spec *spec, unsigned long long value, unsigned int negative, unsigned int base, unsigned int upper)
{

    char number[NUMBERSIZE];
    char *prefix = sign(spec, negative);

    if (spec->alternate && base == 16 && value)
        prefix = (upper) ? "0X" : "0x";

    if (spec->precision >= 0)
        spec->zero = 0;

    putdigits(output, spec, prefix, number, digits(number, value, base, upper, (spec->precision < 0) ? 1 : spec->precision));

}

static void putfloat(struct output *output, struct spec *spec, double value)
{

    char number[NUMBERSIZE * 2];
    int precision = (spec->precision < 0) ? 6 : spec->precision;
    unsigned int negative = value < 0;
    unsigned long long scale = 1;
    unsigned long long whole;
    unsigned long long fraction;
    unsigned int count = 0;
    int i;

    if (precision > 9)
        precision = 9;

    for (i = 0; i < precision; i++)
        scale *= 10;

    if (negative)
        value = -value;

    whole = (unsigned long long)value;
    fraction = (unsigned long long)((value - whole) * scale + 0.5);

    if (fraction >= scale)
    {

        whole++;
        fraction -= scale;

    }

    if (precision)
    {

        count = digits(number, fraction, 10, 0, precision);
        number[count++] = '.';

    }

    count += digits(number + count, whole, 10, 0, 1);

    putdigits(output, spec, sign(spec, negative), number, count);

}

static void putstring(struct output *output, struct spec *spec, const char *in)
{

    int length = 0;
    int i;

    if (!in)
        in = "(null)";

    while (in[length] && (spec->precision < 0 || length < spec->precision))
        length++;

    if (!spec->left)
        pad(output, ' ', spec->width - length);

    for (i = 0; i < length; i++)
        put(output, in[i]);

    if (spec->left)
        pad(output, ' ', spec->width - length);

}

int vsnprintf(char *out, size_t count, const char *fmt, va_list args)
{

    struct output output;
    size_t i;

    output.out = out;
    output.count = count;
    output.offset = 0;

    for (i = 0; fmt[i]; i++)
    {

        struct spec spec;
        unsigned int length = 0;
        char c;

        if (fmt[i] != '%')
        {

            put(&output, fmt[i]);

            continue;

        }

        buffer_clear(&spec, sizeof (struct spec));

        spec.precision = -1;

        for (i++; (c = fmt[i]); i++)
        {

            if (c == '-')
                spec.left = 1;
            else if (c == '0')
                spec.zero = 1;
            else if (c == '+')
                spec.plus = 1;
            else if (c == ' ')
                spec.space = 1;
            else if (c == '#')
                spec.alternate = 1;
            else
                break;

        }

        if (fmt[i] == '*')
        {

            spec.width = va_arg(args, int);
            i++;

            if (spec.width < 0)
            {

                spec.left = 1;
                spec.width = -spec.width;

            }

        }

        for (; isdigit((unsigned char)fmt[i]); i++)
            spec.width = spec.width * 10 + fmt[i] - '0';

        if (fmt[i] == '.')
        {

            spec.precision = 0;
            i++;

            if (fmt[i] == '*')
            {

                spec.precision = va_arg(args, int);
                i++;

            }

            for (; isdigit((unsigned char)fmt[i]); i++)
                spec.precision = spec.precision * 10 + fmt[i] - '0';

        }

        for (; fmt[i] == 'l' || fmt[i] == 'h' || fmt[i] == 'z'; i++)
        {

            if (fmt[i] == 'l')
                length++;

        }

        if (spec.left)
            spec.zero = 0;

        switch (fmt[i])
        {

        case 'd':
        case 'i':
            {

                long long value = (length > 1) ? va_arg(args, long long) : (length) ? va_arg(args, long) : va_arg(args, int);

                putinteger(&output, &spec, (value < 0) ? -(unsigned long long)value : (unsigned long long)value, value < 0, 10, 0);

            }

            break;

        case 'u':
        case 'x':
        case 'X':
        case 'o':
            {

                unsigned long long value = (length > 1) ? va_arg(args, unsigned long long) : (length) ? va_arg(args, unsigned long) : va_arg(args, unsigned int);
                unsigned int base = (fmt[i] == 'u') ? 10 : (fmt[i] == 'o') ? 8 : 16;

                putinteger(&output, &spec, value, 0, base, fmt[i] == 'X');

            }

            break;

        case 'p':
            spec.alternate = 1;

            putinteger(&output, &spec, (unsigned long)va_arg(args, void *), 0, 16, 0);

            break;

        case 'f':
        case 'F':
        case 'g':
        case 'e':
            putfloat(&output, &spec, va_arg(args, double));

            break;

        case 'c':
            {

                char text[2];

                text[0] = (char)va_arg(args, int);
                text[1] = '\0';

                putstring(&output, &spec, text);

            }

            break;

        case 's':
            putstring(&output, &spec, va_arg(args, const char *));

            break;

        case '%':
            put(&output, '%');

            break;

        case '\0':
            i--;

            break;

        }

    }

    if (count)
        out[(output.offset < count) ? output.offset : count - 1] = '\0';

    return output.offset;

}

int vsprintf(char *out, const char *fmt, va_list args)
{

    return vsnprintf(out, 0x7FFFFFFF, fmt, args);

}

int snprintf(char *out, size_t count, const char *fmt, ...)
{

    va_list args;
    int result;

    va_start(args, fmt);

    result = vsnprintf(out, count, fmt, args);

    va_end(args);

    return result;

}

int sprintf(char *out, const char *fmt, ...)
{

    va_list args;
    int result;

    va_start(args, fmt);

    result = vsprintf(out, fmt, args);

    va_end(args);

    return result;

}

/* what goes to stderr is kept, so the last error can be reported when the game exits; nothing else is shown */
int vfprintf(FILE *file, const char *fmt, va_list args)
{

    char text[ERRORSIZE];
    int result = vsnprintf(text, ERRORSIZE, fmt, args);

    if (file == port_stderr)
        strcpy(lasterror, text);

    return result;

}

int fprintf(FILE *file, const char *fmt, ...)
{

    va_list args;
    int result;

    va_start(args, fmt);

    result = vfprintf(file, fmt, args);

    va_end(args);

    return result;

}

int printf(const char *fmt, ...)
{

    va_list args;
    int result;

    va_start(args, fmt);

    result = vfprintf(port_stdout, fmt, args);

    va_end(args);

    return result;

}

int puts(const char *in)
{

    return 0;

}

int fputc(int c, FILE *file)
{

    return c;

}

int fputs(const char *in, FILE *file)
{

    return fprintf(file, "%s", in);

}

char *port_lasterror(void)
{

    return lasterror;

}

static unsigned int inset(const char *set, unsigned int count, char c)
{

    unsigned int negate = (set[0] == '^');
    unsigned int i;

    for (i = negate; i < count; i++)
    {

        if (set[i] == c)
            return !negate;

    }

    return negate;

}

/* the conversions the game asks for: d i u x o f s c and [...], with widths and * */
int sscanf(const char *in, const char *fmt, ...)
{

    va_list args;
    unsigned int count = 0;
    size_t f;
    size_t i = 0;

    va_start(args, fmt);

    for (f = 0; fmt[f]; f++)
    {

        unsigned int suppress = 0;
        unsigned int length = 0;
        int width = 0;
        char c;

        if (isspace((unsigned char)fmt[f]))
        {

            while (isspace((unsigned char)in[i]))
                i++;

            continue;

        }

        if (fmt[f] != '%' || fmt[f + 1] == '%')
        {

            if (fmt[f] == '%')
                f++;

            if (in[i] != fmt[f])
                break;

            i++;

            continue;

        }

        f++;

        if (fmt[f] == '*')
        {

            suppress = 1;
            f++;

        }

        for (; isdigit((unsigned char)fmt[f]); f++)
            width = width * 10 + fmt[f] - '0';

        for (; fmt[f] == 'l' || fmt[f] == 'h'; f++)
        {

            if (fmt[f] == 'l')
                length++;

        }

        c = fmt[f];

        if (c != 'c' && c != '[')
        {

            while (isspace((unsigned char)in[i]))
                i++;

        }

        if (!in[i])
            break;

        if (c == 'd' || c == 'i' || c == 'u' || c == 'x' || c == 'o')
        {

            unsigned int base = (c == 'i') ? 0 : (c == 'x') ? 16 : (c == 'o') ? 8 : 10;
            char *end;
            long value = strtol(in + i, &end, base);

            if (end == in + i)
                break;

            i = end - in;

            if (!suppress)
            {

                if (length)
                    va_arg(args, long *)[0] = value;
                else
                    va_arg(args, int *)[0] = value;

            }

        }

        else if (c == 'f' || c == 'e' || c == 'g')
        {

            char *end;
            double value = strtod(in + i, &end);

            if (end == in + i)
                break;

            i = end - in;

            if (!suppress)
            {

                if (length)
                    va_arg(args, double *)[0] = value;
                else
                    va_arg(args, float *)[0] = value;

            }

        }

        else if (c == 's' || c == 'c' || c == '[')
        {

            char *out = (suppress) ? 0 : va_arg(args, char *);
            const char *set = fmt + f + 1;
            unsigned int setcount = 0;
            int k = 0;

            if (c == '[')
            {

                if (set[0] == '^')
                    setcount++;

                if (set[setcount] == ']')
                    setcount++;

                while (set[setcount] && set[setcount] != ']')
                    setcount++;

                f += setcount + 1;

            }

            if (!width)
                width = (c == 'c') ? 1 : 0x7FFFFFFF;

            while (in[i] && k < width)
            {

                if (c == 's' && isspace((unsigned char)in[i]))
                    break;

                if (c == '[' && !inset(set, setcount, in[i]))
                    break;

                if (out)
                    out[k] = in[i];

                i++;
                k++;

            }

            if (!k)
                break;

            if (out && c != 'c')
                out[k] = '\0';

        }

        else
        {

            break;

        }

        if (!suppress)
            count++;

    }

    va_end(args);

    return count;

}

double sqrt(double x)
{

    double result;

    __asm__ ("fsqrt" : "=t" (result) : "0" (x));

    return result;

}

float sqrtf(float x)
{

    return sqrt(x);

}

double sin(double x)
{

    double result;

    __asm__ ("fsin" : "=t" (result) : "0" (x));

    return result;

}

double cos(double x)
{

    double result;

    __asm__ ("fcos" : "=t" (result) : "0" (x));

    return result;

}

double tan(double x)
{

    double result;

    __asm__ ("fptan\n\tfstp %%st(0)" : "=t" (result) : "0" (x));

    return result;

}

double atan2(double y, double x)
{

    double result;

    __asm__ ("fpatan" : "=t" (result) : "0" (x), "u" (y) : "st(1)");

    return result;

}

double atan(double x)
{

    return atan2(x, 1.0);

}

double asin(double x)
{

    return atan2(x, sqrt(1.0 - x * x));

}

double fabs(double x)
{

    return (x < 0) ? -x : x;

}

double floor(double x)
{

    double whole = (double)(long long)x;

    return (whole > x) ? whole - 1 : whole;

}

double ceil(double x)
{

    double whole = (double)(long long)x;

    return (whole < x) ? whole + 1 : whole;

}

/* x to the power y as 2 to the power y * log2(x), only for a positive x */
double pow(double x, double y)
{

    double exponent;
    double whole;
    double result;

    if (x <= 0)
        return 0;

    __asm__ ("fyl2x" : "=t" (exponent) : "0" (x), "u" (y) : "st(1)");

    whole = floor(exponent);

    __asm__ ("f2xm1" : "=t" (result) : "0" (exponent - whole));
    __asm__ ("fscale" : "=t" (result) : "0" (result + 1.0), "u" (whole));

    return result;

}

unsigned long long __udivdi3(unsigned long long n, unsigned long long d)
{

    return divide(n, d).quotient;

}

unsigned long long __umoddi3(unsigned long long n, unsigned long long d)
{

    return divide(n, d).remainder;

}

long long __divdi3(long long n, long long d)
{

    unsigned long long quotient = divide((n < 0) ? -(unsigned long long)n : (unsigned long long)n, (d < 0) ? -(unsigned long long)d : (unsigned long long)d).quotient;

    return ((n < 0) != (d < 0)) ? -(long long)quotient : (long long)quotient;

}

long long __moddi3(long long n, long long d)
{

    unsigned long long remainder = divide((n < 0) ? -(unsigned long long)n : (unsigned long long)n, (d < 0) ? -(unsigned long long)d : (unsigned long long)d).remainder;

    return (n < 0) ? -(long long)remainder : (long long)remainder;

}

time_t time(time_t *out)
{

    time_t value = SDL_GetTicks() / 1000;

    if (out)
        out[0] = value;

    return value;

}

struct tm *localtime(const time_t *in)
{

    return &emptytime;

}

/* a mem: file of our own holds the heap, named doom and the first free number */
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

        cstring_write_fmt(name, 32, 0, "doom%u\\0", &i);

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

    heap = 0;
    memid = 0;

}

