#include <kernel/kernel.h>
#include <kernel/arch/arm/arch.h>

static struct arch_arm arm;

void arch_init()
{

    for (;;);

    kernel_init(&arm.base);

}

