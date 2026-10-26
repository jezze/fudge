#include <fudge.h>
#include <kernel.h>
#include "udebug.h"
#include "cpu.h"
#include "isr.h"
#include "mmu.h"
#include "arch.h"

static struct cpu_general registers[POOL_TASKS];
static unsigned short ccrs[POOL_TASKS];
static void (*routines[ARCH_INTERRUPTS])(void);

static unsigned long createtable(unsigned long root, unsigned long mmap)
{

    struct mmap_header *header = (struct mmap_header *)mmap;
    unsigned int size = (root == ARCH_MMU_KERNELBASE) ? ARCH_MMU_KERNELSIZE : ARCH_MMU_TASKSIZE;
    unsigned long taddress = root + MMU_ROOTSIZE + header->ntables * MMU_TABLESIZE;

    if (taddress + MMU_TABLESIZE > root + size)
        return 0;

    buffer_clear((void *)taddress, MMU_TABLESIZE);

    header->ntables++;

    return taddress;

}

static unsigned int maptables(unsigned long root, unsigned long mmap, unsigned long vaddress)
{

    if (!mmu_getpointers(root, vaddress))
    {

        unsigned long taddress = createtable(root, mmap);

        if (!taddress)
            return 0;

        mmu_setpointers(root, vaddress, taddress);

    }

    if (!mmu_getpages(root, vaddress))
    {

        unsigned long taddress = createtable(root, mmap);

        if (!taddress)
            return 0;

        mmu_setpages(root, vaddress, taddress);

    }

    return 1;

}

static unsigned int map(unsigned long root, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int flags)
{

    if (!maptables(root, mmap, vaddress))
        return 0;

    mmu_setpage(root, vaddress, paddress, mmu_pflags(flags));

    return 1;

}

static unsigned int maprange(unsigned long root, unsigned long mmap, unsigned long vaddress, unsigned long paddress, unsigned int size, unsigned int flags)
{

    unsigned long offset = vaddress & (MMU_PAGESIZE - 1);
    unsigned int i;

    for (i = 0; i < offset + size; i += MMU_PAGESIZE)
    {

        if (!map(root, mmap, vaddress - offset + i, paddress + i, flags))
            return 0;

    }

    return 1;

}

static unsigned int mapentry(unsigned long root, unsigned long mmap, struct mmap_entry *entry)
{

    if (maprange(root, mmap, entry->vaddress, entry->paddress, entry->size, entry->flags))
    {

        switch (entry->type)
        {

        case MMAP_TYPE_ZERO:
            buffer_clear((void *)entry->vaddress, entry->size);

            break;

        case MMAP_TYPE_BINARY:
            if (entry->fsize)
                buffer_copy((void *)entry->vaddress, (void *)entry->fbase, entry->fsize);

            cpu_invalidatecode();

            if (entry->msize > entry->fsize)
                buffer_clear((void *)(entry->vaddress + entry->fsize), entry->msize - entry->fsize);

            break;

        }

        return 1;

    }

    return 0;

}

static unsigned int mapmem(unsigned long root, unsigned long vaddress, struct mmap_entry *entry)
{

    unsigned long paddress = memsrv_getpage(entry->paddress, vaddress - entry->vaddress);

    return paddress && map(root, KERNEL_VMMAP, vaddress & ~(unsigned long)MMU_PAGEMASK, paddress, entry->flags);

}

/* a task shares the kernel's pointer tables, so kernel mappings made after it started are copied on demand */
static unsigned int copytable(unsigned long root, unsigned long vaddress)
{

    unsigned long pointers = mmu_getpointers(ARCH_MMU_KERNELBASE, vaddress);

    if (!pointers || mmu_getpointers(root, vaddress))
        return 0;

    mmu_setpointers(root, vaddress, pointers);

    return 1;

}

/* clears the pages in the running task, for memsrv */
static void unmap(unsigned long vaddress, unsigned int size)
{

    unsigned long root = cpu_getroot();
    unsigned int i;

    for (i = 0; i < size; i += MMU_PAGESIZE)
    {

        if (mmu_getpages(root, vaddress + i))
            mmu_setpage(root, vaddress + i, 0, 0);

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

            buffer_copy((void *)(unsigned long)(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask), (void *)ARCH_MMU_KERNELBASE, MMU_ROOTSIZE);
            mapentry(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * ntask, ARCH_MMAP_BASE + MMAP_SIZE * ntask, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MMAP_BASE + MMAP_SIZE * ntask, KERNEL_VMMAP, MMAP_SIZE, MMAP_FLAG_WRITEABLE));
            buffer_clear(&registers[ntask], sizeof (struct cpu_general));

            ccrs[ntask] = 0;

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

static void setroot(unsigned long root)
{

    if (cpu_getroot() != root)
        cpu_setroot(root);

}

/* saves the running task from the frame and fills it with the next one, or the idle loop */
static unsigned int schedule(struct cpu_frame *frame)
{

    struct core *core = kernel_getcore();

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(&registers[core->itask], &frame->general, sizeof (struct cpu_general));

        ccrs[core->itask] = frame->sr & CPU_SR_CCR;
        task->thread.ip = frame->pc;
        task->thread.sp = frame->usp.value;

    }

    kernel_schedule(core);

    if (core->itask)
    {

        struct task *task = pool_gettask(core->itask);

        buffer_copy(&frame->general, &registers[core->itask], sizeof (struct cpu_general));

        frame->sr = ccrs[core->itask];
        frame->pc = task->thread.ip;
        frame->usp.value = task->thread.sp;

        setroot(ARCH_MMU_TASKBASE + ARCH_MMU_TASKSIZE * core->itask);

    }

    else
    {

        frame->sr = CPU_SR_SUPERVISOR;
        frame->pc = (unsigned long)cpu_halt;

        setroot(ARCH_MMU_KERNELBASE);

    }

    return 1;

}

static unsigned int fault(struct cpu_frame *frame)
{

    struct core *core = kernel_getcore();

    if (core->itask && !(frame->sr & CPU_SR_SUPERVISOR))
    {

        kernel_kill(core->itask);

        return schedule(frame);

    }

    debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "Kernel fault");

    for (;;);

}

static unsigned int pagefault(unsigned long vaddress)
{

    unsigned long root = cpu_getroot();
    unsigned int found = 0;

    if (mmu_getpage(root, vaddress) & MMU_PFLAG_PRESENT)
        return 0;

    if (root != ARCH_MMU_KERNELBASE)
    {

        struct mmap_entry *entry = mmap_find((struct mmap_header *)KERNEL_VMMAP, vaddress);

        if (entry)
            found = (entry->type == MMAP_TYPE_MEM) ? mapmem(root, vaddress, entry) : mapentry(root, KERNEL_VMMAP, entry);

    }

    if (!found)
        found = copytable(root, vaddress);

    return found;

}

void arch_kmap(unsigned int paddress, unsigned int vaddress, unsigned int size, unsigned int flags)
{

    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate((struct mmap_header *)ARCH_MMAP_BASE, MMAP_TYPE_NORMAL, paddress, vaddress, size, flags));

}

void arch_setinterrupt(unsigned int level, void (*routine)(void))
{

    if (level < ARCH_INTERRUPTS)
        routines[level] = routine;

}

void arch_leave(void)
{

    struct cpu_frame frame;

    buffer_clear(&frame, sizeof (struct cpu_frame));
    schedule(&frame);
    cpu_leave(&frame);

}

/* the page has to be there and writeable before a writeback goes to it, or the kernel would fault */
static unsigned int canwrite(unsigned long vaddress)
{

    unsigned long root = cpu_getroot();

    if (!(mmu_getpage(root, vaddress) & MMU_PFLAG_PRESENT) && !pagefault(vaddress))
        return 0;

    return !(mmu_getpage(root, vaddress) & MMU_PFLAG_WRITEPROTECT);

}

/* a 68040 leaves writes of finished instructions in the access fault's frame; plain data writes are done here, like Linux does (the first writeback is the faulted access, which the cpu does again) */
static unsigned int writeback(unsigned short status, unsigned int address, unsigned int data)
{

    if (!(status & CPU_WB_VALID) || (status & CPU_WB_TT))
        return 1;

    switch (status & CPU_WB_SIZE)
    {

    case CPU_WB_BYTE:
        if (!canwrite(address))
            return 0;

        *(volatile unsigned char *)address = data;

        return 1;

    case CPU_WB_WORD:
        if (!canwrite(address) || !canwrite(address + 1))
            return 0;

        *(volatile unsigned short *)address = data;

        return 1;

    case CPU_WB_LONG:
        if (!canwrite(address) || !canwrite(address + 3))
            return 0;

        *(volatile unsigned int *)address = data;

        return 1;

    }

    return 0;

}

static unsigned int accessfault040(struct cpu_frame *frame, struct cpu_fault040 *fault)
{

    if (fault->ssw & CPU_SSW_ATC && pagefault(fault->fa))
    {

        cpu_flushtlb();

        if (writeback(fault->wb2s, fault->wb2a, fault->wb2d) && writeback(fault->wb3s, fault->wb3a, fault->wb3d))
        {

            fault->wb2s = 0;
            fault->wb3s = 0;

            return 1;

        }

    }

    debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "Access fault 0x%H4h at 0x%H8u (pc 0x%H8u)", &fault->ssw, &fault->fa, &frame->pc);

    return 0;

}

/* a 68060 (and a 68080) restarts the faulted instruction; a misaligned access may fault on the page after its address */
static unsigned int accessfault060(struct cpu_frame *frame, struct cpu_fault060 *fault)
{

    if (fault->fslw & CPU_FSLW_DESCRIPTOR && (pagefault(fault->fa) || (fault->fslw & CPU_FSLW_MISALIGNED && pagefault((fault->fa & ~MMU_PAGEMASK) + MMU_PAGESIZE))))
    {

        cpu_flushtlb();

        return 1;

    }

    debug_fmt(DEBUG_CRITICAL, __FILE__, __LINE__, "Access fault 0x%H8u at 0x%H8u (pc 0x%H8u)", &fault->fslw, &fault->fa, &frame->pc);

    return 0;

}

unsigned int arch_accessfault(struct cpu_frame *frame)
{

    if ((frame->format & CPU_FORMAT_TYPE) == CPU_FORMAT_ACCESS060 ? accessfault060(frame, &frame->fault.m68060) : accessfault040(frame, &frame->fault.m68040))
        return 0;

    return fault(frame);

}

unsigned int arch_fault(struct cpu_frame *frame)
{

    unsigned int vector = (frame->format & CPU_FORMAT_VECTOR) / 4;

    debug_fmt(DEBUG_ERROR, __FILE__, __LINE__, "Exception %u at 0x%H8u", &vector, &frame->pc);

    return fault(frame);

}

/* an interrupt in the kernel can only land on a handler's first instruction, so it goes back there unless the core was idle */
unsigned int arch_interrupt(struct cpu_frame *frame)
{

    struct core *core = kernel_getcore();
    unsigned int level = (frame->format & CPU_FORMAT_VECTOR) / 4 - 24;

    if (level < ARCH_INTERRUPTS && routines[level])
        routines[level]();

    if (core->itask && (frame->sr & CPU_SR_SUPERVISOR))
        return 0;

    return schedule(frame);

}

unsigned int arch_syscall(struct cpu_frame *frame)
{

    struct core *core = kernel_getcore();
    unsigned int index = frame->general.d0.value;

    frame->general.d0.value = abi_call(index, core->itask, frame->usp.reference);

    /* a module's code was relocated with data writes (call 0x07 is load) */
    if (index == 0x07)
        cpu_invalidatecode();

    return schedule(frame);

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
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_MEM_BASE, ARCH_MEM_BASE, ARCH_MEM_SIZE, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));
    mapentry(ARCH_MMU_KERNELBASE, ARCH_MMAP_BASE, mmap_allocate(header, MMAP_TYPE_NORMAL, ARCH_TTY_BASE, ARCH_TTY_BASE, 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE));

}

void arch_setup1(void)
{

    resource_setup();
    udebug_setup();
    buffer_clear((void *)ARCH_MMU_KERNELBASE, MMU_ROOTSIZE);
    setupmmap();
    mailbox_setup();
    kernel_setup();
    abi_setup();
    abi_setcallback(0x03, spawn);

}

void arch_setup2(void)
{

    cpu_setroot(ARCH_MMU_KERNELBASE);
    cpu_settc(CPU_TC_ENABLE);
    cpu_invalidatecode();
    cpu_setcacr(CPU_CACR_INSTRUCTION);
    cpu_setvbr(isr_vectors);

    isr_stack = ARCH_KERNEL_STACKBASE + ARCH_KERNEL_STACKSIZE;

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

