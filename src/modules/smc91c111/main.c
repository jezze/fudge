#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/ethernet.h>
#include <modules/icp/icp.h>

#define PACKETSIZE                      2048
#define REG_BANK                        0x0E
#define REG0_TCR                        0x00
#define REG0_TCR_TXENABLE               (1 << 0)
#define REG0_TCR_PAD                    (1 << 7)
#define REG0_RCR                        0x04
#define REG0_RCR_RXENABLE               (1 << 8)
#define REG0_RCR_STRIPCRC               (1 << 9)
#define REG0_RCR_RESET                  (1 << 15)
#define REG1_ADDRESS                    0x04
#define REG1_CONTROL                    0x0C
#define REG1_CONTROL_AUTORELEASE        (1 << 11)
#define REG2_MMU                        0x00
#define REG2_MMU_ALLOCATE               (1 << 5)
#define REG2_MMU_RESET                  (2 << 5)
#define REG2_MMU_RELEASERX              (4 << 5)
#define REG2_MMU_ENQUEUE                (6 << 5)
#define REG2_PACKET                     0x02
#define REG2_ALLOCATION                 0x03
#define REG2_ALLOCATION_FAILED          0xC0
#define REG2_RXFIFO                     0x05
#define REG2_RXFIFO_EMPTY               (1 << 7)
#define REG2_POINTER                    0x06
#define REG2_POINTER_AUTOINCREMENT      (1 << 14)
#define REG2_POINTER_READ               (1 << 13)
#define REG2_POINTER_RECEIVE            (1 << 15)
#define REG2_DATA                       0x08
#define REG2_INTERRUPT                  0x0C
#define REG2_INTERRUPT_RECEIVE          (1 << 0)
#define REG2_MASK                       0x0D
#define STATUS_ODD                      (1 << 12)
#define CONTROL_ODD                     (1 << 5)

static struct base_driver driver;
static struct ethernet_interface ethernetinterface;
static volatile unsigned char *regs;
static unsigned short rx[PACKETSIZE / 2];

static void setbank(unsigned int bank)
{

    *(volatile unsigned short *)(regs + REG_BANK) = bank;

}

static unsigned short read16(unsigned int offset)
{

    return *(volatile unsigned short *)(regs + offset);

}

static void write16(unsigned int offset, unsigned short value)
{

    *(volatile unsigned short *)(regs + offset) = value;

}

static void handleirq(unsigned int irq)
{

    setbank(2);

    while (!(regs[REG2_RXFIFO] & REG2_RXFIFO_EMPTY))
    {

        unsigned short status;
        unsigned int count;
        unsigned int i;

        write16(REG2_POINTER, REG2_POINTER_RECEIVE | REG2_POINTER_AUTOINCREMENT | REG2_POINTER_READ);

        status = read16(REG2_DATA);
        count = (read16(REG2_DATA) & 0x07FF) - 6;

        if (status & STATUS_ODD)
            count++;

        if (count <= PACKETSIZE)
        {

            for (i = 0; i < (count + 1) / 2; i++)
                rx[i] = read16(REG2_DATA);

            ethernet_notifydata(&ethernetinterface, rx, count);

        }

        regs[REG2_MMU] = REG2_MMU_RELEASERX;

    }

}

static void ethernetinterface_getmac(unsigned char address[6])
{

    unsigned int i;

    setbank(1);

    for (i = 0; i < 6; i++)
        address[i] = regs[REG1_ADDRESS + i];

    setbank(2);

}

/* status, byte count, the data in 16-bit words and a control word that carries an odd last byte */
static unsigned int ethernetinterface_send(void *buffer, unsigned int count)
{

    unsigned char *data = buffer;
    unsigned int even = count & ~1;
    unsigned int i;

    if (count > PACKETSIZE - 6)
        return 0;

    setbank(2);

    regs[REG2_MMU] = REG2_MMU_ALLOCATE;

    if (regs[REG2_ALLOCATION] & REG2_ALLOCATION_FAILED)
        return 0;

    regs[REG2_PACKET] = regs[REG2_ALLOCATION];

    write16(REG2_POINTER, REG2_POINTER_AUTOINCREMENT);
    write16(REG2_DATA, 0);
    write16(REG2_DATA, even + 6);

    for (i = 0; i < even; i += 2)
        write16(REG2_DATA, data[i] | (data[i + 1] << 8));

    write16(REG2_DATA, (count & 1) ? data[count - 1] | (CONTROL_ODD << 8) : 0);

    regs[REG2_MMU] = REG2_MMU_ENQUEUE;

    return count;

}

static void driver_init(unsigned int id)
{

    ethernet_initinterface(&ethernetinterface, id, ethernetinterface_getmac, ethernetinterface_send);

}

static unsigned int driver_match(unsigned int id)
{

    return id == ICP_ETHERNET;

}

static void driver_reset(unsigned int id)
{

    regs = (volatile unsigned char *)icp_getbase(id);

    arch_kmap(icp_getbase(id), icp_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    setbank(0);
    write16(REG0_RCR, REG0_RCR_RESET);
    write16(REG0_RCR, 0);
    setbank(1);
    write16(REG1_CONTROL, read16(REG1_CONTROL) | REG1_CONTROL_AUTORELEASE);
    setbank(2);
    regs[REG2_MMU] = REG2_MMU_RESET;
    setbank(0);
    write16(REG0_TCR, REG0_TCR_TXENABLE | REG0_TCR_PAD);
    write16(REG0_RCR, REG0_RCR_RXENABLE | REG0_RCR_STRIPCRC);
    setbank(2);

}

static void driver_attach(unsigned int id)
{

    ethernet_registerinterface(&ethernetinterface);
    icp_setroutine(icp_getirq(id), handleirq);

    regs[REG2_MASK] = REG2_INTERRUPT_RECEIVE;

}

static void driver_detach(unsigned int id)
{

    regs[REG2_MASK] = 0;

    ethernet_unregisterinterface(&ethernetinterface);
    icp_unsetroutine(icp_getirq(id), handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "smc91c111", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, ICP_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, ICP_BUS);

}
