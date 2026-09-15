#include <fudge.h>
#include <kernel.h>
#include <kernel/x86/gdt.h>
#include <kernel/x86/idt.h>
#include <kernel/x86/tss.h>
#include <kernel/x86/cpu.h>
#include <kernel/x86/arch.h>
#include <modules/base/driver.h>
#include <modules/block/block.h>
#include <modules/arch/x86/ide/ide.h>
#include <modules/arch/x86/pic/pic.h>
#include <modules/arch/x86/apic/apic.h>

static void *blockbuffer;
static struct base_driver driver;
static struct block_interface blockinterface;

static void handleirq(unsigned int irq)
{

    unsigned char status = ide_getstatus(blockinterface.id);
    struct block_session *session = &blockinterface.sessions[0];

    if (status & 1)
        return;

    if (session->source)
    {

        switch (session->type)
        {

        case 1:
            ide_rblock(blockinterface.id, (char *)blockbuffer + session->offset);

            session->offset += 512;

            break;

        case 2:
            if (session->offset < session->count)
            {

                ide_wblock(blockinterface.id, (char *)blockbuffer + session->offset);

                session->offset += 512;

            }

            break;

        }

    }

    if (session->offset == session->count)
        block_session_done(&blockinterface, session);

}

static void blockinterface_oninfo(struct event_blockinfo *blockinfo)
{

    blockinfo->buffer = ARCH_MEM_BASE;
    blockinfo->buffersize = 0x8000;
    blockinfo->blocksize = 512;

}

static void blockinterface_onblockreadrequest(struct block_session *session)
{

    ide_rpio48(blockinterface.id, session->count / 512, session->start / 512);

}

static void blockinterface_onblockwriterequest(struct block_session *session)
{

    ide_wpio48(blockinterface.id, session->count / 512, session->start / 512);

    if (ide_wait(blockinterface.id))
    {

        ide_wblock(blockinterface.id, blockbuffer);

        session->offset = 512;

    }

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, blockinterface_oninfo, blockinterface_onblockreadrequest, blockinterface_onblockwriterequest);

}

static unsigned int driver_match(unsigned int id)
{

    return id == IDE_PM;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    unsigned short irq = ide_getirq(id);

    block_registerinterface(&blockinterface);
    pic_setroutine(irq, handleirq);
    apic_setroutine(irq, handleirq);

}

static void driver_detach(unsigned int id)
{

    unsigned short irq = ide_getirq(id);

    block_unregisterinterface(&blockinterface);
    pic_unsetroutine(irq);
    apic_unsetroutine(irq);

}

void module_init(void)
{

    blockbuffer = (void *)ARCH_MEM_BASE;

    arch_kmap(ARCH_MEM_BASE, ARCH_MEM_BASE, 0x8000, MMAP_FLAG_WRITEABLE | MMAP_FLAG_USERMODE);
    base_initdriver(&driver, "ata", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, IDE_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, IDE_BUS);

}

