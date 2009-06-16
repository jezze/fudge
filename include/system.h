#ifndef SYSTEM_H
#define SYSTEM_H

#include <config.h>
#include <string.h>

typedef struct registers
{

	uint32_t gs, fs, es, ds;
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, cs, eflags, useresp, ss;

} registers_t;

#define PANIC(x) kpanic(x, __FILE__, __LINE__)
#define ASSERT(x) ((x) ? (void)0 : kpanic_assert(__FILE__, __LINE__, #x))

extern void *memcpy(void *, const void *, int32_t);
extern void *memset(void *, int8_t, int32_t);
extern uint16_t *memsetw(uint16_t *, uint16_t, int32_t);
extern uint8_t inb(uint16_t);
extern uint16_t inw(uint16_t);
extern void outb(uint16_t, uint8_t);
extern void cli();
extern void sli();
extern void kpanic(char *message, char *file, uint32_t line);
extern void kpanic_assert(char *file, uint32_t line, char *desc);

#endif

