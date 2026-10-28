#include <fudge.h>
#include <kernel.h>
#include "udebug.h"
#include "cpu.h"
#include "isr.h"
#include "mmu.h"
#include "arch.h"

static struct cpu_general registers[POOL_TASKS];
static struct cpu_interrupt interrupts[POOL_TASKS];
static unsigned int scratch[3][ARCH_SCRATCHSIZE];
static void (*irqroutine)(void);

static unsigned long createtable(unsigned long directory, unsigned long mmap)
{

    struct mmap_header *header = (struct mmap_header *)mmap;
    unsigned int size = (directory == ARCH_MMU_KERNELBASE) ? ARCH_MMU_KERNELSIZE : ARCH_MMU_TASKSIZE;
    unsigned long taddress = directory + MMU_PDSIZE + header->ntables * MMU_PTSIZE;

    if (taddress + MMU_PTSIZE > directory + size)
        return 0;

    buffer_clear((void *)taddress, MMU_PTSIZE);

    header->ntables++;

    return taddress;

}

static unsigned int maptables(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned int flags)
{

    if (!mmu_gettable(directory, vaddress))
    {

        unsigned long taddress = createtable(directory, mmap);

        if (!taddress)
            return 0;

        mmu_settable(directory, vaddress, taddress, mmu_tflags(flags));

    }

    return 1;

}

static unsigned int map(unsigned long directory, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    if (!maptables(directory, mmap, vaddress, flags))
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

static unsigned int copytable(unsigned long directory, unsigned long vaddress)
{

    unsigned long table = mmu_gettable(ARCH_MMU_KERNELBASE, vaddress);

    if (!(table & MMU_TFLAG_PRESENT) || (mmu_gettable(directory, vaddress) & MMU_TFLAG_PRESENT))
        return 0;

    mmu_settable(directory, vaddress, table, table);

    return 1;

}

/* clears the pages in the running task, for memsrv */
static void unmap(unsigned long vaddress, unsigned int size)
{

    unsigned long directory = cpu_getttbr();
    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        if (mmu_gettable(directory, vaddress + i) & MMU_TFLAG_PRESENT)
            mmu_setpage(directory, vaddress + i, 0, 0);

    }

    cpu_flushtlb();

}

static unsigned int createtask(unsigned long address)
{

    unsigned int ntask = pool_picktask();

    if (ntask)
    {

        struct mmap_header *header = (struct mmap_header *)(unsigned long)(ARCH_MMAP_BASE + MMAP_SIZE * ntask);
        unsigned int inode = kernel_loadtask(ntask, 0, KERNEL_VSTACK, address, ARCH_MMAP_BASE + MMAP_SIZE * ntask, ARCH_TASK_CODEBASE + TASK_CODESIZE * ntask, ARCH_TASK_STACKBASE + TASK_STACKSIZE * ntask, ARCH_TASK_BUFFERBASE + TASK_BUFFERSIZE * ntask);

        if (inode)
        {

            buffer_copy((void *)(unsigned long)(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask), (void *)ARCH_MMU_KERNELBASE, MMU_PDSIZE);
            mapentry(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, ARCH_MMAP_BASE + MMAP_SIZE * ntask, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE + MMAP_SIZE * ntask, KERNEL_VMMAP, MMAP_SIZE, MMAP_FLAG_WRITEABLE));
            buffer_clear(&registers[ntask], sizeof (struct cpu_general));
            buffer_clear(&interrupts[ntask], sizeof (struct cpu_interrupt));
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
    unsigned long address = kernel_locate(args->target, args->id);

    if (address)
        return createtask(address);

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "spawn failed");

    return 0;

}

static void setdirectory(unsigned long directory)
{

    if (cpu_getttbr() != directory)
        cpu_setttbr(directory);

}

static void schedule(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(&registers[core->itask], general, sizeof (struct cpu_general));

        interrupts[core->itask].cpsr.value = interrupt->cpsr.value;
        interrupts[core->itask].lr.value = interrupt->lr.value;
        task->thread.ip = interrupt->pc.value;
        task->thread.sp = interrupt->sp.value;

    }

    kernel_schedule(core);

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(general, &registers[core->itask], sizeof (struct cpu_general));

        interrupt->cpsr.value = (interrupts[core->itask].cpsr.value & CPU_FLAGS_NZCVQ) | CPU_MODE_USR;
        interrupt->lr.value = interrupts[core->itask].lr.value;
        interrupt->pc.value = task->thread.ip;
        interrupt->sp.value = task->thread.sp;

        setdirectory(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * core->itask);

    }

    else
    {

        interrupt->cpsr.value = CPU_MODE_SVC | CPU_FLAGS_F;
        interrupt->pc.value = (unsigned long)cpu_halt;

        setdirectory(ARCH_MMU_KERNELBASE);

    }

}

static void fault(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    if (core->itask && (interrupt->cpsr.value & CPU_MODE_MASK) == CPU_MODE_USR)
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

    unsigned long directory = cpu_getttbr();
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
        found = copytable(directory, vaddress);

    return found;

}

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags)
{

    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate((struct mmap_header *)ARCH_MMAP_BASE, MMAP_TYPE_NORMAL, paddress, vaddress, size, flags));

}

void arch_setirq(void (*routine)(void))
{

    irqroutine = routine;

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

void arch_undefined(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Undefined instruction at 0x%H8u", &interrupt->pc.value);
    fault(general, interrupt);

}

void arch_prefetchabort(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    if (!pagefault(interrupt->pc.value))
    {

        debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "Prefetch abort at 0x%H8u", &interrupt->pc.value);
        fault(general, interrupt);

    }

}

void arch_dataabort(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    unsigned int status = cpu_getfsr() & MMU_FSR_STATUS;
    unsigned int vaddress = cpu_getfar();

    if (!((status == MMU_FSR_SECTION || status == MMU_FSR_PAGE) && pagefault(vaddress)))
    {

        debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "Data abort %u 0x%H8u at 0x%H8u", &status, &vaddress, &interrupt->pc.value);
        fault(general, interrupt);

    }

}

void arch_irq(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    if (irqroutine)
        irqroutine();

    arch_resume(general, interrupt);

}

void arch_syscall(struct cpu_general *general, struct cpu_interrupt *interrupt)
{

    struct core *core = kernel_getcore();

    general->r0.value = abi_call(general->r12.value, core->itask, interrupt->sp.reference);

    arch_resume(general, interrupt);

}

static void setupmmap(void)
{

    struct mmap_header *header = (struct mmap_header *)ARCH_MMAP_BASE;

    mmap_initheader(header);
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_VECTOR_BASE, ARCH_VECTOR_BASE, ARCH_VECTOR_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODEBASE, ARCH_KERNEL_CODESIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKBASE, ARCH_KERNEL_STACKSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE, ARCH_MMAP_BASE, ARCH_MMAP_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELBASE, ARCH_MMU_KERNELSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMU_TASKBASE, ARCH_MMU_TASKBASE, ARCH_MMU_TASKSIZE * POOL_TASKS, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MAILBOX_BASE, ARCH_MAILBOX_BASE, ARCH_MAILBOX_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_TASK_BUFFERBASE, ARCH_TASK_BUFFERBASE, ARCH_TASK_BUFFERSIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_TASK_CODEBASE, ARCH_TASK_CODEBASE, ARCH_TASK_CODESIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MEM_BASE, ARCH_MEM_BASE, ARCH_MEM_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_UART1_BASE, ARCH_UART1_BASE, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));

}

static void setupvectors(void)
{

    buffer_copy((void *)ARCH_VECTOR_BASE, isr_vectors, ISR_VECTORSIZE);
    cpu_setmodestack(CPU_MODE_IRQ, scratch[0]);
    cpu_setmodestack(CPU_MODE_ABT, scratch[1]);
    cpu_setmodestack(CPU_MODE_UND, scratch[2]);

    isr_stack = ARCH_KERNEL_STACKBASE + ARCH_KERNEL_STACKSIZE;

}

void arch_setup1(void)
{

    resource_setup();
    udebug_setup();
    buffer_clear((void *)ARCH_MMU_KERNELBASE, MMU_PDSIZE);
    setupmmap();
    mailbox_setup();
    kernel_setup();
    abi_setup();
    abi_setcallback(0x03, spawn);

}

void arch_setup2(void)
{

    cpu_setttbr(ARCH_MMU_KERNELBASE);
    mmu_enable();
    setupvectors();
    pool_setup(ARCH_MAILBOX_BASE);
    memsrv_setup(ARCH_MEM_BASE, unmap);

}

void arch_runinit(unsigned int address)
{

    struct core *core = kernel_getcore();
    unsigned int target = createtask(address);

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

