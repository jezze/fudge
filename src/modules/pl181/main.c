#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/block.h>
#include <modules/icp/icp.h>

#define BLOCKSIZE                       512
#define POLLS                           0x100000
#define REG_POWER                       0x00
#define REG_POWER_ON                    0x03
#define REG_CLOCK                       0x04
#define REG_CLOCK_ENABLE                (1 << 8)
#define REG_ARGUMENT                    0x08
#define REG_COMMAND                     0x0C
#define REG_COMMAND_RESPONSE            (1 << 6)
#define REG_COMMAND_LONG                (1 << 7)
#define REG_COMMAND_ENABLE              (1 << 10)
#define REG_RESPONSE0                   0x14
#define REG_DATATIMER                   0x24
#define REG_DATALENGTH                  0x28
#define REG_DATACTRL                    0x2C
#define REG_DATACTRL_ENABLE             (1 << 0)
#define REG_DATACTRL_READ               (1 << 1)
#define REG_DATACTRL_BLOCK512           (9 << 4)
#define REG_STATUS                      0x34
#define REG_STATUS_CMDCRCFAIL           (1 << 0)
#define REG_STATUS_DATACRCFAIL          (1 << 1)
#define REG_STATUS_CMDTIMEOUT           (1 << 2)
#define REG_STATUS_DATATIMEOUT          (1 << 3)
#define REG_STATUS_CMDRESPEND           (1 << 6)
#define REG_STATUS_CMDSENT              (1 << 7)
#define REG_STATUS_DATAEND              (1 << 8)
#define REG_STATUS_TXFIFOFULL           (1 << 16)
#define REG_STATUS_RXDATAAVAILABLE      (1 << 21)
#define REG_STATUS_ERRORS               (REG_STATUS_CMDCRCFAIL | REG_STATUS_DATACRCFAIL | REG_STATUS_CMDTIMEOUT | REG_STATUS_DATATIMEOUT)
#define REG_CLEAR                       0x38
#define REG_CLEAR_ALL                   0x000007FF
#define REG_FIFO                        0x80
#define SD_GOIDLE                       0
#define SD_ALLSENDCID                   2
#define SD_SENDRCA                      3
#define SD_SELECT                       7
#define SD_SENDIFCOND                   8
#define SD_SENDCSD                      9
#define SD_SETBLOCKLEN                  16
#define SD_READBLOCK                    17
#define SD_WRITEBLOCK                   24
#define SD_APPCMD                       55
#define SD_APPSENDOPCOND                41
#define SD_OCR_READY                    (1 << 31)
#define SD_OCR_HIGHCAPACITY             (1 << 30)
#define SD_OCR_REQUEST                  0x40FF8000

static struct base_driver driver;
static struct block_interface blockinterface;
static volatile unsigned int *regs;
static unsigned int highcapacity;
static unsigned int rca;

static unsigned int waitstatus(unsigned int mask)
{

    unsigned int i;

    for (i = 0; i < POLLS; i++)
    {

        unsigned int status = regs[REG_STATUS / 4];

        if (status & REG_STATUS_ERRORS)
            return 0;

        if (status & mask)
            return 1;

    }

    return 0;

}

/* sends a command and waits for its response, or only for it to be sent when there is none */
static unsigned int command(unsigned int index, unsigned int argument, unsigned int flags)
{

    unsigned int status;

    regs[REG_CLEAR / 4] = REG_CLEAR_ALL;
    regs[REG_ARGUMENT / 4] = argument;
    regs[REG_COMMAND / 4] = index | flags | REG_COMMAND_ENABLE;

    status = waitstatus((flags & REG_COMMAND_RESPONSE) ? REG_STATUS_CMDRESPEND : REG_STATUS_CMDSENT);

    regs[REG_CLEAR / 4] = REG_STATUS_CMDRESPEND | REG_STATUS_CMDSENT;

    return status;

}

static unsigned int response(unsigned int index)
{

    return regs[REG_RESPONSE0 / 4 + index];

}

static void startdata(unsigned int flags)
{

    regs[REG_DATATIMER / 4] = 0xFFFFFFFF;
    regs[REG_DATALENGTH / 4] = BLOCKSIZE;
    regs[REG_DATACTRL / 4] = REG_DATACTRL_ENABLE | REG_DATACTRL_BLOCK512 | flags;

}

static unsigned int readblock(unsigned int block, unsigned int *buffer)
{

    unsigned int count = 0;
    unsigned int i;

    startdata(REG_DATACTRL_READ);

    if (!command(SD_READBLOCK, (highcapacity) ? block : block * BLOCKSIZE, REG_COMMAND_RESPONSE))
        return 0;

    for (i = 0; i < POLLS && count < BLOCKSIZE / 4; i++)
    {

        unsigned int status = regs[REG_STATUS / 4];

        if (status & REG_STATUS_ERRORS)
            return 0;

        if (status & REG_STATUS_RXDATAAVAILABLE)
            buffer[count++] = regs[REG_FIFO / 4];

    }

    return count == BLOCKSIZE / 4;

}

static unsigned int writeblock(unsigned int block, unsigned int *buffer)
{

    unsigned int count = 0;
    unsigned int i;

    if (!command(SD_WRITEBLOCK, (highcapacity) ? block : block * BLOCKSIZE, REG_COMMAND_RESPONSE))
        return 0;

    startdata(0);

    for (i = 0; i < POLLS && count < BLOCKSIZE / 4; i++)
    {

        unsigned int status = regs[REG_STATUS / 4];

        if (status & REG_STATUS_ERRORS)
            return 0;

        if (!(status & REG_STATUS_TXFIFOFULL))
            regs[REG_FIFO / 4] = buffer[count++];

    }

    return count == BLOCKSIZE / 4 && waitstatus(REG_STATUS_DATAEND);

}

/* the session's data is copied block by block between the card's FIFO and the task's buffer */
static void blockinterface_startsession(struct block_session *session)
{

    unsigned int *buffer = (unsigned int *)kernel_getbuffer(session->source);
    unsigned int count = (session->count > TASK_BUFFERSIZE) ? TASK_BUFFERSIZE : session->count;
    unsigned int i;

    count -= count % BLOCKSIZE;

    if (!buffer || !rca || session->start % BLOCKSIZE)
        count = 0;

    for (i = 0; i < count; i += BLOCKSIZE)
    {

        unsigned int block = (session->start + i) / BLOCKSIZE;
        unsigned int done = (session->type == BLOCK_TYPE_WRITE) ? writeblock(block, buffer + i / 4) : readblock(block, buffer + i / 4);

        if (!done)
            break;

    }

    session->count = i;

    block_session_done(&blockinterface, session);

}

/* blocks from the CSD, version 1 (standard capacity) or 2 (high capacity) */
static unsigned int getblockcount(void)
{

    if ((response(0) >> 30) == 1)
        return ((((response(1) & 0x3F) << 16) | (response(2) >> 16)) + 1) * 1024;

    return (((((response(1) & 0x3FF) << 2) | (response(2) >> 30)) + 1) << (((response(2) >> 15) & 7) + 2)) << ((response(1) >> 16) & 0x0F) >> 9;

}

static void setupcard(void)
{

    unsigned int i;

    rca = 0;

    command(SD_GOIDLE, 0, 0);
    command(SD_SENDIFCOND, 0x1AA, REG_COMMAND_RESPONSE);

    for (i = 0; i < 1000; i++)
    {

        if (!command(SD_APPCMD, 0, REG_COMMAND_RESPONSE) || !command(SD_APPSENDOPCOND, SD_OCR_REQUEST, REG_COMMAND_RESPONSE))
            return;

        if (response(0) & SD_OCR_READY)
            break;

    }

    highcapacity = response(0) & SD_OCR_HIGHCAPACITY;

    if (!command(SD_ALLSENDCID, 0, REG_COMMAND_RESPONSE | REG_COMMAND_LONG))
        return;

    if (!command(SD_SENDRCA, 0, REG_COMMAND_RESPONSE))
        return;

    rca = response(0) & 0xFFFF0000;

    if (!command(SD_SENDCSD, rca, REG_COMMAND_RESPONSE | REG_COMMAND_LONG))
    {

        rca = 0;

        return;

    }

    blockinterface.blockcount = getblockcount();

    if (!command(SD_SELECT, rca, REG_COMMAND_RESPONSE) || !command(SD_SETBLOCKLEN, BLOCKSIZE, REG_COMMAND_RESPONSE))
        rca = 0;

}

static void driver_init(unsigned int id)
{

    block_initinterface(&blockinterface, id, BLOCKSIZE, blockinterface_startsession);

}

static unsigned int driver_match(unsigned int id)
{

    return id == ICP_MMCI;

}

static void driver_reset(unsigned int id)
{

    regs = (volatile unsigned int *)icp_getbase(id);

    arch_kmap(icp_getbase(id), icp_getbase(id), 0x1000, MMAP_FLAG_GLOBAL | MMAP_FLAG_WRITEABLE);

    regs[REG_POWER / 4] = REG_POWER_ON;
    regs[REG_CLOCK / 4] = REG_CLOCK_ENABLE;

    setupcard();

}

static void driver_attach(unsigned int id)
{

    if (rca)
        block_registerinterface(&blockinterface);

}

static void driver_detach(unsigned int id)
{

    if (rca)
        block_unregisterinterface(&blockinterface);

}

void module_init(void)
{

    base_initdriver(&driver, "pl181", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, ICP_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, ICP_BUS);

}
