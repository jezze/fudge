#ifndef SYSTEM_H
#define SYSTEM_H

#include <types.h>
#include <string.h>

typedef struct registers
{

	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;

} registers_t;

#define PANIC(x) kernel_panic(x, __FILE__, __LINE__)
#define ASSERT(x) ((x) ? (void)0 : kernel_assert(__FILE__, __LINE__, #x))

extern void cli();
extern void sli();
extern void kernel_panic(char *message, char *file, uint32_t line);
extern void kernel_assert(char *file, uint32_t line, char *desc);

#endif

