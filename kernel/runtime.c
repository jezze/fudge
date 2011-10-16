#include <lib/memory.h>
#include <lib/string.h>
#include <kernel/vfs.h>
#include <kernel/modules.h>
#include <kernel/arch/x86/mmu.h>
#include <modules/elf/elf.h>
#include <kernel/runtime.h>

struct runtime_task runtimeTasks[8];

void runtime_init()
{

}

