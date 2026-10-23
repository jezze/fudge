#include <fudge.h>
#include <kernel.h>
#include <cpu.h>
#include <arch.h>
#include <modules/driver.h>
#include <modules/keyboard.h>
#include "amiga.h"

#define EXTENDED                        0x100
#define KEYS                            0x68
#define HANDSHAKE                       64
#define REG_CIAA                        0x00BFE000
#define REG_SDR                         0xC01
#define REG_CRA                         0xE01
#define REG_CRA_SPMODE                  0x40

static struct base_driver driver;
static struct keyboard_interface keyboardinterface;
static volatile unsigned char *cia = (volatile unsigned char *)REG_CIAA;

/* amiga raw key codes to scancode set 1; keys a pc keyboard lacks (help, the keypad's brackets) are left out */
static unsigned short scancodes[KEYS] = {
    0x29, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x2B, 0x00, 0x52,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x00, 0x4F, 0x50, 0x51,
    0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x2B, 0x00, 0x4B, 0x4C, 0x4D,
    0x56, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x00, 0x53, 0x47, 0x48, 0x49,
    0x39, 0x0E, 0x0F, EXTENDED | 0x1C, 0x1C, 0x01, EXTENDED | 0x53, 0x00, 0x00, 0x00, 0x4A, 0x00, EXTENDED | 0x48, EXTENDED | 0x50, EXTENDED | 0x4D, EXTENDED | 0x4B,
    0x3B, 0x3C, 0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, 0x00, 0x00, EXTENDED | 0x35, 0x37, 0x4E, 0x00,
    0x2A, 0x36, 0x3A, 0x1D, 0x38, EXTENDED | 0x38, EXTENDED | 0x5B, EXTENDED | 0x5C
};

/* the keyboard waits until the sdr line is held low for at least 85 microseconds; each cia access takes over one */
static void handshake(void)
{

    unsigned int i;

    cia[REG_CRA] |= REG_CRA_SPMODE;

    for (i = 0; i < HANDSHAKE; i++)
        cia[REG_SDR];

    cia[REG_CRA] &= ~REG_CRA_SPMODE;

}

/* the keyboard sends each code inverted and rotated left, with bit 7 set on release */
static void handleirq(unsigned int irq)
{

    unsigned char data = ~cia[REG_SDR];
    unsigned char code = (data >> 1) | (data << 7);
    unsigned short scancode;

    handshake();

    if ((code & 0x7F) >= KEYS)
        return;

    scancode = scancodes[code & 0x7F];

    if (!scancode)
        return;

    if (code & 0x80)
    {

        if (scancode & EXTENDED)
            keyboard_notifyrelease(&keyboardinterface, 0xE0);

        keyboard_notifyrelease(&keyboardinterface, (scancode & 0xFF) | 0x80);

    }

    else
    {

        if (scancode & EXTENDED)
            keyboard_notifypress(&keyboardinterface, 0xE0);

        keyboard_notifypress(&keyboardinterface, scancode & 0xFF);

    }

}

static void driver_init(unsigned int id)
{

    keyboard_initinterface(&keyboardinterface, id);

}

static unsigned int driver_match(unsigned int id)
{

    return id == AMIGA_KEYBOARD;

}

static void driver_reset(unsigned int id)
{

}

static void driver_attach(unsigned int id)
{

    keyboard_registerinterface(&keyboardinterface);
    amiga_setciaroutine(AMIGA_CIA_SP, handleirq);

}

static void driver_detach(unsigned int id)
{

    keyboard_unregisterinterface(&keyboardinterface);
    amiga_unsetciaroutine(AMIGA_CIA_SP, handleirq);

}

void module_init(void)
{

    base_initdriver(&driver, "amiga-keyboard", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver, AMIGA_BUS);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver, AMIGA_BUS);

}

