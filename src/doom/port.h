#ifndef PORT_H
#define PORT_H

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef __SIZE_TYPE__ size_t;
typedef __PTRDIFF_TYPE__ ptrdiff_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef long off_t;
typedef __builtin_va_list va_list;
typedef struct port_file FILE;

struct stat
{

    unsigned int st_mode;
    long st_size;

};


#define va_start(list, last)            __builtin_va_start(list, last)
#define va_arg(list, type)              __builtin_va_arg(list, type)
#define va_end(list)                    __builtin_va_end(list)
#define va_copy(out, in)                __builtin_va_copy(out, in)
#define offsetof(type, member)          __builtin_offsetof(type, member)
#define NULL                            ((void *)0)
#define EOF                             (-1)
#define SEEK_SET                        0
#define SEEK_CUR                        1
#define SEEK_END                        2
#define O_RDONLY                        0
#define O_WRONLY                        1
#define O_RDWR                          2
#define O_CREAT                         0x40
#define O_TRUNC                         0x200
#define O_BINARY                        0
#define R_OK                            4
#define W_OK                            2
#define F_OK                            0
#define PATH_MAX                        256
#define CHAR_BIT                        8
#define SCHAR_MIN                       (-128)
#define SCHAR_MAX                       127
#define UCHAR_MAX                       255
#define SHRT_MIN                        (-32768)
#define SHRT_MAX                        32767
#define USHRT_MAX                       65535
#define INT_MIN                         (-INT_MAX - 1)
#define INT_MAX                         0x7FFFFFFF
#define UINT_MAX                        0xFFFFFFFFU
#define LONG_MIN                        (-LONG_MAX - 1L)
#define LONG_MAX                        __LONG_MAX__
#define ULONG_MAX                       (LONG_MAX * 2UL + 1UL)
#define LLONG_MAX                       __LONG_LONG_MAX__
#define M_PI                            3.14159265358979323846
#define assert(expression)              ((void)0)
#define stdout                          (port_stdout)
#define stderr                          (port_stderr)
#define S_ISDIR(mode)                   (((mode) & 0xF000) == 0x4000)

extern int errno;
extern FILE *port_stdout;
extern FILE *port_stderr;

void *memcpy(void *out, const void *in, size_t count);
void *memmove(void *out, const void *in, size_t count);
void *memset(void *out, int value, size_t count);
int memcmp(const void *in1, const void *in2, size_t count);
void *memchr(const void *in, int c, size_t count);
size_t strlen(const char *in);
char *strcpy(char *out, const char *in);
char *strncpy(char *out, const char *in, size_t count);
char *strcat(char *out, const char *in);
char *strncat(char *out, const char *in, size_t count);
int strcmp(const char *in1, const char *in2);
int strncmp(const char *in1, const char *in2, size_t count);
int strcasecmp(const char *in1, const char *in2);
int strncasecmp(const char *in1, const char *in2, size_t count);
char *strchr(const char *in, int c);
char *strstr(const char *in, const char *needle);
char *strtok(char *in, const char *delimiters);
char *strerror(int error);
long strtol(const char *in, char **end, int base);
unsigned long strtoul(const char *in, char **end, int base);
double strtod(const char *in, char **end);
int atoi(const char *in);
int abs(int value);
long labs(long value);
int toupper(int c);
int tolower(int c);
int isspace(int c);
int isdigit(int c);
int isxdigit(int c);
int isalpha(int c);
int isalnum(int c);
int isupper(int c);
int islower(int c);
void *malloc(size_t size);
void *realloc(void *pointer, size_t size);
void free(void *pointer);
char *getenv(const char *name);
int system(const char *command);
int atexit(void (*function)(void));
void exit(int status) __attribute__((noreturn));
void abort(void) __attribute__((noreturn));
int printf(const char *fmt, ...);
int fprintf(FILE *file, const char *fmt, ...);
int sprintf(char *out, const char *fmt, ...);
int snprintf(char *out, size_t count, const char *fmt, ...);
int vsnprintf(char *out, size_t count, const char *fmt, va_list args);
int vsprintf(char *out, const char *fmt, va_list args);
int vfprintf(FILE *file, const char *fmt, va_list args);
int sscanf(const char *in, const char *fmt, ...);
FILE *fopen(const char *path, const char *mode);
int fclose(FILE *file);
size_t fread(void *out, size_t size, size_t count, FILE *file);
size_t fwrite(const void *in, size_t size, size_t count, FILE *file);
int fseek(FILE *file, long offset, int whence);
long ftell(FILE *file);
char *fgets(char *out, int count, FILE *file);
int fgetc(FILE *file);
int fputc(int c, FILE *file);
int feof(FILE *file);
void setbuf(FILE *file, char *buffer);
int remove(const char *path);
int access(const char *path, int mode);
int stat(const char *path, struct stat *out);
int unlink(const char *path);
int isatty(int fd);
char *mktemp(char *template);
int open(const char *path, int flags, ...);
int close(int fd);
int read(int fd, void *out, size_t count);
off_t lseek(int fd, off_t offset, int whence);
double sqrt(double x);
double sin(double x);
double cos(double x);
double tan(double x);
double atan(double x);
double asin(double x);
double atan2(double y, double x);
double pow(double x, double y);
double floor(double x);
double fabs(double x);
unsigned int SDL_GetTicks(void);
char *port_lasterror(void);
unsigned int port_init(char *path);
void port_destroy(void);

#endif
