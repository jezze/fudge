#include <kernel/arch/x86/cpu.h>
#include <kernel/arch/x86/fpu.h>

void fpu_init()
{

    cpu_enable_fpu();

}

