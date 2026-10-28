#include <fudge.h>
#include <kernel.h>
#include "udebug.h"
#include "cpu.h"
#include "trap.h"
#include "mmu.h"
#include "arch.h"

static struct cpu_general registers[POOL_TASKS];
static void (*routines[ARCH_INTERRUPTS])(void);

static unsigned long createtable(unsigned long directory, unsigned long mmap)
{

    struct mmap_header *header = (struct mmap_header *)mmap;
    unsigned int size = (directory == ARCH_MMU_KERNELBASE) ? ARCH_MMU_KERNELSIZE : ARCH_MMU_TASKSIZE;
    unsigned long taddress = directory + MMU_TABLESIZE + header->ntables * MMU_TABLESIZE;

    if (taddress + MMU_TABLESIZE > directory + size)
        return 0;

    buffer_clear((void *)taddress, MMU_TABLESIZE);

    header->ntables++;

    return taddress;

}

static unsigned int maptables(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned int level, unsigned int flags)
{

    unsigned int i;

    for (i = MMU_LEVELS - 1; i >= level; i--)
    {

        if (!mmu_gettable(directory, vaddress, i))
        {

            unsigned long taddress = createtable(directory, mmap);

            if (!taddress)
                return 0;

            mmu_settable(directory, vaddress, i, taddress, mmu_tflags(flags));

        }

    }

    return 1;

}

static unsigned int map(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    if (!maptables(directory, mmap, vaddress, 1, flags))
        return 0;

    mmu_setpage(directory, vaddress, paddress, mmu_pflags(flags));

    return 1;

}

static unsigned int maprange(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int size, unsigned int flags)
{

    unsigned long offset = vaddress & (MMU_PAGESIZE - 1);
    unsigned int i;

    for (i = 0; i < offset + size; i += MMU_PAGESIZE)
    {

        if (!map(directory, mmap, vaddress - offset + i, paddress + i, flags))
            return 0;

    }

    return 1;

}

static unsigned int mapentry(unsigned long directory, unsigned long mmap, struct mmap_entry *entry)
{

    if (maprange(directory, mmap, entry->vaddress, entry->paddress, entry->size, entry->flags))
    {

        cpu_flushtlb();

        switch (entry->type)
        {

        case MMAP_TYPE_ZERO:
            buffer_clear((void *)entry->vaddress, entry->size);

            break;

        }

        return 1;

    }

    return 0;

}

static unsigned int mapmem(unsigned long directory, unsigned long vaddress, struct mmap_entry *entry)
{

    unsigned long paddress = memsrv_getpage(entry->paddress, vaddress - entry->vaddress);

    return paddress && map(directory, KERNEL_VMMAP, vaddress & ~(unsigned long)MMU_PAGEMASK, paddress, entry->flags);

}

static unsigned int copytable(unsigned long directory, unsigned long mmap, unsigned long vaddress)
{

    unsigned long table = mmu_gettable(ARCH_MMU_KERNELBASE, vaddress, 1);

    if (!(table & MMU_TFLAG_PRESENT) || (mmu_gettable(directory, vaddress, 1) & MMU_TFLAG_PRESENT))
        return 0;

    if (!maptables(directory, mmap, vaddress, 2, 0))
        return 0;

    mmu_settable(directory, vaddress, 1, table, table);

    return 1;

}

static unsigned long getdirectory(void)
{

    return (cpu_getsatp() & 0x00000FFFFFFFFFFFUL) << 12;

}

static void setdirectory(unsigned long directory)
{

    if (getdirectory() != directory)
        cpu_setsatp(MMU_SATP_SV39 | (directory >> 12));

}

/* clears the pages in the running task, for memsrv */
static void unmap(unsigned long vaddress, unsigned int size)
{

    unsigned long directory = getdirectory();
    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
        mmu_setpage(directory, vaddress + i, 0, 0);

    cpu_flushtlb();

}

static unsigned int createtask(unsigned long base, unsigned int id)
{

    unsigned int ntask = pool_picktask();

    if (ntask)
    {

        struct mmap_header *header = (struct mmap_header *)(unsigned long)(ARCH_MMAP_BASE + MMAP_SIZE * ntask);
        unsigned int inode = kernel_loadtask(ntask, 0, KERNEL_VSTACK, base, id, ARCH_MMAP_BASE + MMAP_SIZE * ntask, ARCH_TASK_CODEBASE + TASK_CODESIZE * ntask, ARCH_TASK_STACKBASE + TASK_STACKSIZE * ntask, ARCH_TASK_BUFFERBASE + TASK_BUFFERSIZE * ntask);

        if (inode)
        {

            unsigned long vaddress;

            buffer_clear((void *)(unsigned long)(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask), MMU_TABLESIZE);

            for (vaddress = 0; vaddress < ARCH_MMU_LIMIT; vaddress += MMU_PAGESIZE * MMU_ENTRIES)
                copytable(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, ARCH_MMAP_BASE + MMAP_SIZE * ntask, vaddress);

            mapentry(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, ARCH_MMAP_BASE + MMAP_SIZE * ntask, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE + MMAP_SIZE * ntask, KERNEL_VMMAP, MMAP_SIZE, MMAP_FLAG_WRITEABLE));
            buffer_clear(&registers[ntask], sizeof (struct cpu_general));
            kernel_starttask(ntask);

            return inode;

        }

        pool_unpicktask(ntask);

    }

    return 0;

}

static unsigned int spawn(unsigned int itask, void *stack)
{

    struct {void *caller; unsigned int target; unsigned int id;} *args = stack;
    unsigned long base = memsrv_getbase(args->target, args->id);
    unsigned int inode = (base) ? createtask(base, args->id) : 0;

    if (!inode)
        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "spawn failed");

    return inode;

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(&registers[core->itask], general, sizeof (struct cpu_general));

        task->thread.ip = interrupt->pc.value;
        task->thread.sp = interrupt->sp.value;

    }

    kernel_schedule(core);

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(general, &registers[core->itask], sizeof (struct cpu_general));

        interrupt->status.value = CPU_STATUS_SPIE | CPU_STATUS_SUM;
        interrupt->pc.value = task->thread.ip;
        interrupt->sp.value = task->thread.sp;

        setdirectory(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * core->itask);

    }

    else
    {

        interrupt->status.value = CPU_STATUS_SPP | CPU_STATUS_SPIE | CPU_STATUS_SUM;
        interrupt->pc.value = (unsigned long)cpu_halt;
        interrupt->sp.value = trap_stack;

        setdirectory(ARCH_MMU_KERNELBASE);

    }

}

static void fault(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask && !(interrupt->status.value & CPU_STATUS_SPP))
    {

        kernel_kill(core->itask);
        arch_resume(general, interrupt);

        return;

    }

    debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "Kernel fault");

    for (;;);

}

static unsigned int pagefault(unsigned long vaddress)
{

    unsigned long directory = getdirectory();
    unsigned int found = 0;

    if (mmu_getpage(directory, vaddress) & MMU_PFLAG_PRESENT)
        return 0;

    if (directory != ARCH_MMU_KERNELBASE)
    {

        struct mmap_entry *entry = mmap_find((struct mmap_header *)KERNEL_VMMAP, vaddress);

        if (entry)
            found = (entry->type == MMAP_TYPE_MEM) ? mapmem(directory, vaddress, entry) : mapentry(directory, KERNEL_VMMAP, entry);

    }

    if (!found)
        found = copytable(directory, KERNEL_VMMAP, vaddress);

    cpu_flushtlb();

    return found;

}

static void debugaddress(char *name, unsigned long cause, unsigned long value, unsigned long pc)
{

    unsigned int causelow = cause;
    unsigned int high = value >> 32;
    unsigned int low = value;
    unsigned int pchigh = pc >> 32;
    unsigned int pclow = pc;

    debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "%s %u 0x%H8u%H8u at 0x%H8u%H8u", name, &causelow, &high, &low, &pchigh, &pclow);

}

void arch_kmap(unsigned long paddress, unsigned long vaddress, unsigned int size, unsigned int flags)
{

    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate((struct mmap_header *)ARCH_MMAP_BASE, MMAP_TYPE_NORMAL, paddress, vaddress, size, flags));

}

void arch_setinterrupt(unsigned int index, void (*routine)(void))
{

    if (index < ARCH_INTERRUPTS)
    {

        routines[index] = routine;

        if (routine)
            cpu_enableinterrupt(1UL << index);
        else
            cpu_disableinterrupt(1UL << index);

    }

}

void arch_resume(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    schedule(general, interrupt);

}

void arch_leave(void)
{

    struct cpu_frame frame;

    buffer_clear(&frame, sizeof (struct cpu_frame));
    schedule(&frame.general, &frame.interrupt);
    cpu_leave(&frame);

}

void arch_trap(struct cpu_general *general, struct cpu_interrupt *interrupt, unsigned long cause, unsigned long value)
{

    if (cause & CPU_CAUSE_INTERRUPT)
    {

        unsigned int index = cause & (ARCH_INTERRUPTS - 1);

        if (routines[index])
            routines[index]();

        arch_resume(general, interrupt);

        return;

    }

    switch (cause)
    {

    case CPU_CAUSE_USERCALL:
        interrupt->pc.value += 4;
        general->a0.value = abi_call(general->a7.value, kernel_getcore()->itask, interrupt->sp.reference);

        arch_resume(general, interrupt);

        break;

    case CPU_CAUSE_FETCHPAGE:
    case CPU_CAUSE_LOADPAGE:
    case CPU_CAUSE_STOREPAGE:
        if (!pagefault(value))
        {

            debugaddress("Page fault", cause, value, interrupt->pc.value);
            fault(general, interrupt);

        }

        break;

    default:
        debugaddress("Exception", cause, value, interrupt->pc.value);
        fault(general, interrupt);

        break;

    }

}

static void setupmmap(void)
{

    struct mmap_header *header = (struct mmap_header *)ARCH_MMAP_BASE;

    mmap_initheader(header);
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODESIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE, ARCH_MMAP_BASE, ARCH_MMAP_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_TASKBASE, ARCH_MMU_TASKBASE, ARCH_MMU_TASKSIZE * POOL_TASKS, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MAILBOX_BASE, ARCH_MAILBOX_BASE, ARCH_MAILBOX_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_TASK_BUFFERBASE, ARCH_TASK_BUFFERBASE, ARCH_TASK_BUFFERSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_TASK_CODEBASE, ARCH_TASK_CODEBASE, ARCH_TASK_CODESIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MEM_BASE, ARCH_MEM_BASE, ARCH_MEM_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_UART_BASE, ARCH_UART_BASE, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));

}

void arch_setup1(void)
{

    resource_setup();
    udebug_setup();
    buffer_clear((void *)ARCH_MMU_KERNELBASE, MMU_TABLESIZE);
    setupmmap();
    mailbox_setup();
    kernel_setup();
    abi_setup();
    abi_setcallback(0x03, spawn);

}

void arch_setup2(void)
{

    trap_stack = ARCH_KERNEL_STACKBASE + ARCH_KERNEL_STACKSIZE;

    cpu_settrap(trap_entry);
    cpu_setstatus(CPU_STATUS_SUM);
    setdirectory(ARCH_MMU_KERNELBASE);
    pool_setup(ARCH_MAILBOX_BASE);
    memsrv_setup(ARCH_MEM_BASE, unmap);

}

void arch_runinit(unsigned long address)
{

    struct core *core = kernel_getcore();
    unsigned int target = createtask(address, 0);

    if (core)
        core_register(core);

    if (target)
    {

        kernel_place(0, target, EVENT_MAIN, 0, 0);
        arch_leave();

    }

    else
    {

        debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "spawn failed");

    }

}

