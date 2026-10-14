#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/audio.h>
#include <modules/io/io.h>
#include <modules/pci/pci.h>

#define VENDOR                          0x8086
#define NAM_RESET                       0x00
#define NAM_MASTER                      0x02
#define NAM_PCMOUT                      0x18
#define NAM_EXTID                       0x28
#define NAM_EXTID_VRA                   (1 << 0)
#define NAM_EXTCTRL                     0x2A
#define NAM_EXTCTRL_VRA                 (1 << 0)
#define NAM_FRONTRATE                   0x2C
#define NABM_POBDBAR                    0x10
#define NABM_POCIV                      0x14
#define NABM_POLVI                      0x15
#define NABM_POSR                       0x16
#define NABM_POSR_DCH                   (1 << 0)
#define NABM_POSR_CLEAR                 0x1C
#define NABM_POCR                       0x1B
#define NABM_POCR_RPBM                  (1 << 0)
#define NABM_POCR_RR                    (1 << 1)
#define NABM_GLOBCNT                    0x2C
#define NABM_GLOBCNT_COLD               (1 << 1)
#define NABM_GLOBSTA                    0x30
#define NABM_GLOBSTA_PCR                (1 << 8)
#define ENTRIES                         32
#define ENTRYSIZE                       MESSAGE_SIZE
#define LIMIT_POLL                      100000

struct entry
{

    unsigned int address;
    unsigned short samples;
    unsigned short flags;

};

static struct base_driver driver;
static struct audio_interface audiointerface;
static struct spinlock spinlock;
static unsigned short nam;
static unsigned short nabm;
static unsigned int vra;
static unsigned int running;
static unsigned char memory[ENTRIES * sizeof (struct entry) + ENTRIES * ENTRYSIZE + 8];
static struct entry *entries;
static unsigned char *buffers;

static unsigned int reset(void)
{

    unsigned int i;

    io_outd(nabm + NABM_GLOBCNT, NABM_GLOBCNT_COLD);

    for (i = 0; i < LIMIT_POLL; i++)
    {

        if (io_ind(nabm + NABM_GLOBSTA) & NABM_GLOBSTA_PCR)
            break;

    }

    if (i == LIMIT_POLL)
        return 0;

    io_outw(nam + NAM_RESET, 0);
    io_outw(nam + NAM_MASTER, 0x0000);
    io_outw(nam + NAM_PCMOUT, 0x0808);

    vra = io_inw(nam + NAM_EXTID) & NAM_EXTID_VRA;

    if (vra)
        io_outw(nam + NAM_EXTCTRL, io_inw(nam + NAM_EXTCTRL) | NAM_EXTCTRL_VRA);

    io_outb(nabm + NABM_POCR, NABM_POCR_RR);

    for (i = 0; i < LIMIT_POLL && (io_inb(nabm + NABM_POCR) & NABM_POCR_RR); i++);

    io_outd(nabm + NABM_POBDBAR, (unsigned long)entries);

    return 1;

}

static unsigned int audiointerface_setformat(unsigned int rate, unsigned int channels, unsigned int bits)
{

    if (channels != 2 || bits != 16)
        return 0;

    if (rate != 48000 && !vra)
        return 0;

    if (rate < 8000 || rate > 48000)
        return 0;

    spinlock_acquire(&spinlock);

    if (vra)
    {

        io_outw(nam + NAM_FRONTRATE, rate);

        rate = io_inw(nam + NAM_FRONTRATE);

    }

    audiointerface.rate = rate;

    spinlock_release(&spinlock);

    return 1;

}

static unsigned int audiointerface_write(void *buffer, unsigned int count)
{

    unsigned int next;

    /* whole frames only: two channels of 16 bits */
    count &= ~3;

    if (!count)
        return 1;

    spinlock_acquire(&spinlock);

    if (!running)
    {

        next = 0;

    }

    else
    {

        next = (io_inb(nabm + NABM_POLVI) + 1) % ENTRIES;

        /* the buffer after the last valid one is still being played, so every buffer is full */
        if (!(io_inw(nabm + NABM_POSR) & NABM_POSR_DCH) && next == io_inb(nabm + NABM_POCIV))
        {

            spinlock_release(&spinlock);

            return 0;

        }

    }

    buffer_copy(buffers + next * ENTRYSIZE, buffer, count);

    entries[next].address = (unsigned long)(buffers + next * ENTRYSIZE);
    entries[next].samples = count / 2;
    entries[next].flags = 0;

    io_outw(nabm + NABM_POSR, NABM_POSR_CLEAR);
    io_outb(nabm + NABM_POLVI, next);

    if (!running)
    {

        io_outb(nabm + NABM_POCR, NABM_POCR_RPBM);

        running = 1;

    }

    spinlock_release(&spinlock);

    return 1;

}

static void driver_init(unsigned int id)
{

    /* the device wants the buffer list on an 8 byte boundary, and the module loader doesn't keep the alignment of static data */
    entries = (struct entry *)(((unsigned long)memory + 7) & ~7UL);
    buffers = (unsigned char *)(entries + ENTRIES);

    audio_initinterface(&audiointerface, id, audiointerface_setformat, audiointerface_write);

    audiointerface.rate = 48000;
    audiointerface.channels = 2;
    audiointerface.bits = 16;

}

static unsigned int driver_match(unsigned int id)
{

    return pci_inw(id, PCI_CONFIG_VENDOR) == VENDOR && pci_inb(id, PCI_CONFIG_CLASS) == PCI_CLASS_MEDIA && pci_inb(id, PCI_CONFIG_SUBCLASS) == PCI_CLASS_MEDIA_AUDIO;

}

static void driver_reset(unsigned int id)
{

    nam = pci_getbar(id, 0);
    nabm = pci_getbar(id, 1);

    pci_setmaster(id);

    if (!reset())
        audiointerface.rate = 0;

}

static void driver_attach(unsigned int id)
{

    if (audiointerface.rate)
        audio_registerinterface(&audiointerface);

}

static void driver_detach(unsigned int id)
{

    if (audiointerface.rate)
        audio_unregisterinterface(&audiointerface);

}

void module_init(void)
{

    base_initdriver(&driver, "ac97", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, PCI_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, PCI_BUS);

}
