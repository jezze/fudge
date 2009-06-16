#include <system.h>
#include <syscall.h>
#include <isr.h>
#include <screen.h>

static void *syscalls[3] = {&puts, &puts_hex, &puts_dec}

uint32_t num_syscalls = 3;

void syscall_handler(registers_t *r)
{

	if (r->eax >= num_syscalls)
		return;

	void *location = syscalls[r->eax];

	int ret;

	__asm__ __volatile__ (" \
		push %1; \
		push %2; \
		push %3; \
		push %4; \
		push %5; \
		call %6; \
		pop %%ebx; \
		pop %%ebx; \
		pop %%ebx; \
		pop %%ebx; \
		pop %%ebx; \
	" : "=a" (ret) : "r" (r->edi), "r" (r->esi), "r" (r->edx), "r" (r->ecx), "r" (r->ebx), "r" (location));

	r->eax = ret;

}

void syscalls_init()
{

	isr_register_handler(0x80, &syscall_handler);

}

