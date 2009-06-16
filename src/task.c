#include <system.h>

void switch_to_user_mode()
{

	__asm__ __volatile__ (" \
		cli; \
		mov $0x23, %ax; \
		mov %ax, %ds; \
		mov %ax, %es; \
		mov %ax, %fs; \
		mov %ax, %gs; \

		mov %esp, %eax; \
		pushl $023; \
		pushl %eax; \
		pushf; \
		pop %eax; \
		or %eax, $0x200; \
		push %eax; \
		pushl $0x1B; \
		push $1f; \
		iret; \
		1: \
	");

}


}
