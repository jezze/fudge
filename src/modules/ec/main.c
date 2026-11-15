#include <fudge.h>
#include <kernel.h>
#include <modules/io/io.h>
#include <modules/acpi/acpi.h>
#include <modules/battery.h>

#define FIRMWARES                       1
#define EC_WAIT                         100000
#define EC_FAILED                       0x100
#define EC_STATUS_OBF                   0x01
#define EC_STATUS_IBF                   0x02
#define EC_COMMAND_READ                 0x80
#define EC_COMMAND_WRITE                0x81
#define BATTERY_STATE                   0x07
#define BATTERY_NEGATIVE                0x8000

struct firmware
{

    char *oem;
    unsigned short data;
    unsigned short command;
    unsigned char select;
    unsigned char slot;
    unsigned char status;
    unsigned char present;
    unsigned char acstatus;
    unsigned char ac;
    unsigned char state;
    unsigned char rate;
    unsigned char voltage;
    unsigned char remaining;
    unsigned char full;
    unsigned char design;
    unsigned int unit;

};

static struct firmware firmwares[FIRMWARES] = {
    {"DELL  ", 0x0930, 0x0934, 0x03, 0x01, 0x06, 0x02, 0x00, 0x01, 0x10, 0x12, 0x14, 0x16, 0x1E, 0x20, MESSAGE_BATTERY_MILLIAMPS}
};

static struct firmware *firmware;
static struct spinlock lock;
static struct battery_interface batteryinterface;

static unsigned int waitstatus(unsigned int mask, unsigned int value)
{

    unsigned int i;

    for (i = 0; i < EC_WAIT; i++)
    {

        if ((io_inb(firmware->command) & mask) == value)
            return 1;

    }

    return 0;

}

static unsigned int sendaddress(unsigned int command, unsigned int address)
{

    if (io_inb(firmware->command) & EC_STATUS_OBF)
        io_inb(firmware->data);

    if (!waitstatus(EC_STATUS_IBF, 0))
        return 0;

    io_outb(firmware->command, command);

    if (!waitstatus(EC_STATUS_IBF, 0))
        return 0;

    io_outb(firmware->data, address);

    return 1;

}

static unsigned int readbyte(unsigned int address)
{

    if (!sendaddress(EC_COMMAND_READ, address))
        return EC_FAILED;

    if (!waitstatus(EC_STATUS_OBF, EC_STATUS_OBF))
        return EC_FAILED;

    return io_inb(firmware->data);

}

static unsigned int writebyte(unsigned int address, unsigned int value)
{

    if (!sendaddress(EC_COMMAND_WRITE, address))
        return 0;

    if (!waitstatus(EC_STATUS_IBF, 0))
        return 0;

    io_outb(firmware->data, value);

    return waitstatus(EC_STATUS_IBF, 0);

}

static unsigned int readword(unsigned int address)
{

    unsigned int low = readbyte(address);
    unsigned int high = (low == EC_FAILED) ? EC_FAILED : readbyte(address + 1);

    return (high == EC_FAILED) ? EC_FAILED : low | (high << 8);

}

static unsigned int readbattery(unsigned int address)
{

    unsigned int value = EC_FAILED;

    spinlock_acquire(&lock);

    if (writebyte(firmware->select, firmware->slot))
        value = readword(address);

    spinlock_release(&lock);

    return value;

}

static unsigned int readcapacity(unsigned int address)
{

    unsigned int value = readbattery(address);

    return (value == EC_FAILED) ? MESSAGE_BATTERY_UNKNOWN : value;

}

static unsigned int getpresent(void)
{

    unsigned int value;

    spinlock_acquire(&lock);

    value = readbyte(firmware->status);

    spinlock_release(&lock);

    return value != EC_FAILED && (value & firmware->present);

}

static unsigned int getstate(void)
{

    unsigned int value = readbattery(firmware->state);

    return (value == EC_FAILED) ? 0 : value & BATTERY_STATE;

}

static unsigned int getrate(void)
{

    unsigned int ac;
    unsigned int value = EC_FAILED;

    spinlock_acquire(&lock);

    ac = readbyte(firmware->acstatus);

    if (ac != EC_FAILED && writebyte(firmware->select, firmware->slot))
        value = readword(firmware->rate);

    spinlock_release(&lock);

    if (value == EC_FAILED)
        return MESSAGE_BATTERY_UNKNOWN;

    if (!value)
        return 0;

    if (ac & firmware->ac)
        return (value & BATTERY_NEGATIVE) ? MESSAGE_BATTERY_UNKNOWN : value;

    return (value & BATTERY_NEGATIVE) ? 0x10000 - value : MESSAGE_BATTERY_UNKNOWN;

}

static unsigned int getremaining(void)
{

    return readcapacity(firmware->remaining);

}

static unsigned int getfull(void)
{

    return readcapacity(firmware->full);

}

static unsigned int getdesign(void)
{

    return readcapacity(firmware->design);

}

static unsigned int getvoltage(void)
{

    return readcapacity(firmware->voltage);

}

static struct firmware *findfirmware(void)
{

    struct acpi_sdth *dsdt = acpi_findheader("DSDT", 0);
    unsigned int i;

    if (!dsdt)
        return 0;

    for (i = 0; i < FIRMWARES; i++)
    {

        if (buffer_match(dsdt->oem, firmwares[i].oem, 6) && io_inb(firmwares[i].command) != 0xFF)
            return &firmwares[i];

    }

    return 0;

}

void module_init(void)
{

    spinlock_init(&lock);

    firmware = findfirmware();

    if (firmware)
        battery_initinterface(&batteryinterface, firmware->unit, getpresent, getstate, getrate, getremaining, getfull, getdesign, getvoltage);

}

void module_register(void)
{

    if (firmware)
        battery_registerinterface(&batteryinterface);

}

void module_unregister(void)
{

    if (firmware)
        battery_unregisterinterface(&batteryinterface);

}

