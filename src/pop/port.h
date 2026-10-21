typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned int uint32_t;
typedef signed int int32_t;
typedef unsigned long long uint64_t;
typedef signed long long int64_t;
typedef __SIZE_TYPE__ size_t;
typedef __INTPTR_TYPE__ intptr_t;
typedef __UINTPTR_TYPE__ uintptr_t;
typedef struct port_file FILE;
typedef void *jmp_buf[5];

#ifndef __cplusplus
#define bool                            _Bool
#define true                            1
#define false                           0
#endif
#define NULL                            ((void *)0)
#define INT_MAX                         0x7FFFFFFF
#define INT16_MIN                       (-32768)
#define SEEK_SET                        0
#define SEEK_CUR                        1
#define SEEK_END                        2
#define alloca(size)                    __builtin_alloca(size)
#define setjmp(buffer)                  __builtin_setjmp(buffer)
#define printf(...)                     ((void)0)
#define fprintf(...)                    ((void)0)
#define perror(...)                     ((void)0)

void *memcpy(void *out, const void *in, size_t count);
void *memmove(void *out, const void *in, size_t count);
void *memset(void *out, int value, size_t count);
int memcmp(const void *in1, const void *in2, size_t count);
size_t strlen(const char *in);
char *strcpy(char *out, const char *in);
char *strncpy(char *out, const char *in, size_t count);
int strcmp(const char *in1, const char *in2);
int strncmp(const char *in1, const char *in2, size_t count);
int strncasecmp(const char *in1, const char *in2, size_t count);
int strcasecmp(const char *in1, const char *in2);
char *strchr(const char *in, int c);
char *strrchr(const char *in, int c);
int atoi(const char *in);
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *pointer, size_t size);
void free(void *pointer);
FILE *fopen(const char *path, const char *mode);
size_t fwrite(const void *in, size_t size, size_t count, FILE *file);
size_t fread(void *out, size_t size, size_t count, FILE *file);
int fseek(FILE *file, long offset, int whence);
long ftell(FILE *file);
int fclose(FILE *file);
int remove(const char *path);
unsigned int time(void *unused);
void exit(int status) __attribute__((noreturn));
void longjmp(jmp_buf buffer, int value) __attribute__((noreturn));
int port_exists(const char *path);
int port_isdirectory(const char *path);
unsigned int cstring_write_fmt(void *out, unsigned int count, unsigned int offset, char *fmt, ...);
int port_filesize(FILE *file);
