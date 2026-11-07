#include <fudge.h>
#include <kernel.h>
#include <modules/driver.h>
#include <modules/mouse.h>
#include <modules/pic/pic.h>
#include <modules/apic/apic.h>
#include <modules/gpio/gpio.h>
#include "i2c.h"

#define HIDREGISTERS                    2
#define TIMEOUT                         1000000
#define POLLTIMEOUT                     50000
#define DRAINS                          4
#define RETRIES                         2
#define RESETDELAY                      200000
#define TIMERIRQ                        0
#define USAGES                          16
#define REPORTIDS                       256
#define BUTTONS                         3
#define DUMPLINE                        16
#define HID_POWERON                     0x0800
#define HID_RESET                       0x0100
#define HID_SETREPORT                   0x03
#define HID_REPORTTYPE_FEATURE          0x03
#define HID_INPUTMODE_TOUCHPAD          3
#define HID_SELECT_ALL                  3
#define TAPTICKS                        12
#define PADCOUNTS                       1600
#define WHEELCOUNTS                     24
#define HID_ITEM_SIZE                   0x03
#define HID_ITEM_LONG                   0xFE
#define HID_MAIN_INPUT                  0x80
#define HID_MAIN_COLLECTION             0xA0
#define HID_MAIN_ENDCOLLECTION          0xC0
#define HID_MAIN_FEATURE                0xB0
#define HID_GLOBAL_LOGICALMAX           0x24
#define HID_GLOBAL_USAGEPAGE            0x04
#define HID_GLOBAL_REPORTSIZE           0x74
#define HID_GLOBAL_REPORTID             0x84
#define HID_GLOBAL_REPORTCOUNT          0x94
#define HID_LOCAL_USAGE                 0x08
#define HID_LOCAL_USAGEMIN              0x18
#define HID_LOCAL_USAGEMAX              0x28
#define HID_INPUT_CONSTANT              0x01
#define HID_INPUT_VARIABLE              0x02
#define HID_INPUT_RELATIVE              0x04
#define HID_COLLECTION_APPLICATION      0x01
#define HID_USAGE_MOUSE                 0x00010002
#define HID_USAGE_X                     0x00010030
#define HID_USAGE_Y                     0x00010031
#define HID_USAGE_WHEEL                 0x00010038
#define HID_PAGE_BUTTON                 0x0009
#define HID_USAGE_BUTTON1               0x00090001
#define HID_USAGE_TOUCHPAD              0x000D0005
#define HID_USAGE_FINGER                0x000D0022
#define HID_USAGE_TIPSWITCH             0x000D0042
#define HID_USAGE_CONTACTID             0x000D0051
#define HID_USAGE_INPUTMODE             0x000D0052
#define HID_USAGE_CONTACTCOUNT          0x000D0054
#define HID_USAGE_SURFACESWITCH         0x000D0057
#define HID_USAGE_BUTTONSWITCH          0x000D0058
#define HIDDESC_LENGTH                  30
#define HIDDESC_VERSION                 0x0100

struct hid
{

    unsigned int id;
    unsigned int reportregister;
    unsigned int reportlength;
    unsigned int inputregister;
    unsigned int inputlength;
    unsigned int commandregister;
    unsigned int dataregister;

};

struct field
{

    unsigned int found;
    unsigned int offset;
    unsigned int size;
    unsigned int relative;

};

struct hidmouse
{

    unsigned int found;
    unsigned int usesids;
    unsigned int reportid;
    unsigned int buttoncount;
    unsigned int oldbuttons;
    unsigned int length;
    struct field buttons;
    struct field x;
    struct field y;
    struct field wheel;

};

struct hidpad
{

    unsigned int found;
    unsigned int reportid;
    unsigned int modereport;
    unsigned int selectreport;
    unsigned int xmax;
    unsigned int ymax;
    struct field tip;
    struct field contactid;
    struct field x;
    struct field y;
    struct field contactcount;
    struct field button;

};

struct parser
{

    unsigned int usagepage;
    unsigned int reportsize;
    unsigned int reportcount;
    unsigned int reportid;
    unsigned int usages[USAGES];
    unsigned int nusages;
    unsigned int usagemin;
    unsigned int usagemax;
    unsigned int depth;
    unsigned int mousedepth;
    unsigned int logicalmax;
    unsigned int application;
    unsigned int fingerdepth;
    unsigned int fingerdone;
    unsigned int offsets[REPORTIDS];

};

static struct base_driver driver;
static struct mouse_interface mouseinterface;
static struct i2c_transfer transfer;
static unsigned int unread;
static struct {int x; int y; int wheel;} motion;
static struct hid hid;
static struct hidmouse hidmouse;
static struct hidpad hidpad;
static unsigned int padmode;
static unsigned int readlength;
static unsigned int ticks;
static struct {unsigned int active; unsigned int id; int lastx; int lasty; unsigned int downtick; unsigned int travel; unsigned int fingers; unsigned int button; int accx; int accy; int accw;} touch;
static struct parser parser;
static struct gpio_pad pad;
static unsigned int padinterrupt;
static struct spinlock spinlock;
static unsigned short hidregisters[HIDREGISTERS] = {0x0001, 0x0020};

static unsigned int getinword(struct i2c_transfer *transfer, unsigned int offset)
{

    return transfer->in[offset] | (transfer->in[offset + 1] << 8);

}

static unsigned int readhiddescriptor(struct i2c_transfer *transfer, unsigned int id, unsigned int reg)
{

    transfer->id = id;
    transfer->out[0] = reg;
    transfer->out[1] = reg >> 8;
    transfer->outcount = 2;
    transfer->incount = HIDDESC_LENGTH;
    transfer->abort = 0;
    transfer->timeout = TIMEOUT;

    return i2c_runtransfer(transfer) && getinword(transfer, 0) == HIDDESC_LENGTH && getinword(transfer, 2) == HIDDESC_VERSION;

}

static unsigned int sendcommand(struct i2c_transfer *transfer, unsigned int command)
{

    transfer->id = hid.id;
    transfer->out[0] = hid.commandregister;
    transfer->out[1] = hid.commandregister >> 8;
    transfer->out[2] = command;
    transfer->out[3] = command >> 8;
    transfer->outcount = 4;
    transfer->incount = 0;
    transfer->abort = 0;
    transfer->timeout = TIMEOUT;

    return i2c_runtransfer(transfer);

}

/* SET_REPORT of a one-byte feature report: command register with opcode and report, then the data register, the length, the report id and the value */
static unsigned int setfeature(struct i2c_transfer *transfer, unsigned int reportid, unsigned int value)
{

    unsigned int n = 0;

    transfer->id = hid.id;
    transfer->out[n++] = hid.commandregister;
    transfer->out[n++] = hid.commandregister >> 8;

    if (reportid < 15)
    {

        transfer->out[n++] = (HID_REPORTTYPE_FEATURE << 4) | reportid;
        transfer->out[n++] = HID_SETREPORT;

    }

    else
    {

        transfer->out[n++] = (HID_REPORTTYPE_FEATURE << 4) | 15;
        transfer->out[n++] = HID_SETREPORT;
        transfer->out[n++] = reportid;

    }

    transfer->out[n++] = hid.dataregister;
    transfer->out[n++] = hid.dataregister >> 8;
    transfer->out[n++] = 4;
    transfer->out[n++] = 0;
    transfer->out[n++] = reportid;
    transfer->out[n++] = value;
    transfer->outcount = n;
    transfer->incount = 0;
    transfer->abort = 0;
    transfer->timeout = TIMEOUT;

    return i2c_runtransfer(transfer);

}

static unsigned int readreportdescriptor(struct i2c_transfer *transfer)
{

    transfer->id = hid.id;
    transfer->out[0] = hid.reportregister;
    transfer->out[1] = hid.reportregister >> 8;
    transfer->outcount = 2;
    transfer->incount = hid.reportlength;
    transfer->abort = 0;
    transfer->timeout = TIMEOUT;

    return i2c_runtransfer(transfer);

}

static unsigned int readinput(struct i2c_transfer *transfer)
{

    transfer->id = hid.id;
    transfer->outcount = 0;
    transfer->incount = readlength;
    transfer->abort = 0;
    transfer->timeout = POLLTIMEOUT;

    return i2c_runtransfer(transfer);

}

static unsigned int getitemdata(struct i2c_transfer *transfer, unsigned int offset, unsigned int size)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < size; i++)
        value |= transfer->in[offset + i] << (i * 8);

    return value;

}

static unsigned int getusage(struct parser *parser, unsigned int index)
{

    if (parser->nusages)
        return parser->usages[(index < parser->nusages) ? index : parser->nusages - 1];

    if (parser->usagemax >= parser->usagemin)
        return (parser->usagemin + index <= parser->usagemax) ? parser->usagemin + index : parser->usagemax;

    return 0;

}

static void setfield(struct field *field, unsigned int offset, unsigned int size, unsigned int flags)
{

    if (field->found)
        return;

    field->found = 1;
    field->offset = offset;
    field->size = size;
    field->relative = (flags & HID_INPUT_RELATIVE) ? 1 : 0;

}

/* the touchpad collection of a Precision Touchpad: one finger per report (hybrid reporting), its tip switch, contact id and absolute X/Y, the contact count and the button */
static void parsepadinput(struct parser *parser, struct hidpad *pad, unsigned int base, unsigned int flags)
{

    unsigned int i;

    if (parser->application != HID_USAGE_TOUCHPAD || (flags & HID_INPUT_CONSTANT))
        return;

    if (pad->x.found && pad->reportid != parser->reportid)
        return;

    for (i = 0; i < parser->reportcount; i++)
    {

        unsigned int usage = getusage(parser, i);
        unsigned int offset = base + i * parser->reportsize;

        if (parser->fingerdepth)
        {

            if (usage == HID_USAGE_TIPSWITCH)
                setfield(&pad->tip, offset, parser->reportsize, flags);

            if (usage == HID_USAGE_CONTACTID)
                setfield(&pad->contactid, offset, parser->reportsize, flags);

            if (usage == HID_USAGE_X && !pad->x.found)
            {

                setfield(&pad->x, offset, parser->reportsize, flags);

                pad->xmax = parser->logicalmax;

            }

            if (usage == HID_USAGE_Y && !pad->y.found)
            {

                setfield(&pad->y, offset, parser->reportsize, flags);

                pad->ymax = parser->logicalmax;

            }

        }

        if (usage == HID_USAGE_CONTACTCOUNT)
            setfield(&pad->contactcount, offset, parser->reportsize, flags);

        if (usage == HID_USAGE_BUTTON1)
            setfield(&pad->button, offset, parser->reportsize, flags);

        pad->reportid = parser->reportid;

    }

}

static void parsepadfeature(struct parser *parser, struct hidpad *pad)
{

    unsigned int i;

    for (i = 0; i < parser->reportcount; i++)
    {

        unsigned int usage = getusage(parser, i);

        if (usage == HID_USAGE_INPUTMODE && !pad->modereport)
            pad->modereport = parser->reportid;

        if ((usage == HID_USAGE_SURFACESWITCH || usage == HID_USAGE_BUTTONSWITCH) && !pad->selectreport)
            pad->selectreport = parser->reportid;

    }

}

static void parseinput(struct parser *parser, struct hidmouse *mouse, unsigned int flags)
{

    unsigned int base = parser->offsets[parser->reportid];
    unsigned int i;

    parser->offsets[parser->reportid] += parser->reportsize * parser->reportcount;

    parsepadinput(parser, &hidpad, base, flags);

    if (!parser->mousedepth || (flags & HID_INPUT_CONSTANT))
        return;

    if (mouse->x.found && mouse->reportid != parser->reportid)
        return;

    for (i = 0; i < parser->reportcount; i++)
    {

        unsigned int usage = getusage(parser, i);
        unsigned int offset = base + i * parser->reportsize;

        if ((usage >> 16) == HID_PAGE_BUTTON && parser->reportsize == 1 && (flags & HID_INPUT_VARIABLE))
        {

            if (!mouse->buttons.found)
                setfield(&mouse->buttons, offset, 1, flags);

            if (mouse->buttoncount < BUTTONS && offset == mouse->buttons.offset + mouse->buttoncount)
                mouse->buttoncount++;

        }

        if (usage == HID_USAGE_X)
            setfield(&mouse->x, offset, parser->reportsize, flags);

        if (usage == HID_USAGE_Y)
            setfield(&mouse->y, offset, parser->reportsize, flags);

        if (usage == HID_USAGE_WHEEL)
            setfield(&mouse->wheel, offset, parser->reportsize, flags);

        mouse->reportid = parser->reportid;

    }

}

static void parsereportdescriptor(struct i2c_transfer *transfer, struct parser *parser, struct hidmouse *mouse)
{

    unsigned int offset = 0;

    buffer_clear(parser, sizeof (struct parser));
    buffer_clear(mouse, sizeof (struct hidmouse));
    buffer_clear(&hidpad, sizeof (struct hidpad));

    while (offset < transfer->incount)
    {

        unsigned int prefix = transfer->in[offset];
        unsigned int size = ((prefix & HID_ITEM_SIZE) == HID_ITEM_SIZE) ? 4 : prefix & HID_ITEM_SIZE;
        unsigned int data;

        if (prefix == HID_ITEM_LONG)
        {

            offset += 3 + ((offset + 1 < transfer->incount) ? transfer->in[offset + 1] : 0);

            continue;

        }

        if (offset + 1 + size > transfer->incount)
            break;

        data = getitemdata(transfer, offset + 1, size);

        switch (prefix & ~HID_ITEM_SIZE)
        {

        case HID_GLOBAL_USAGEPAGE:
            parser->usagepage = data;

            break;

        case HID_GLOBAL_REPORTSIZE:
            parser->reportsize = data;

            break;

        case HID_GLOBAL_REPORTCOUNT:
            parser->reportcount = data;

            break;

        case HID_GLOBAL_LOGICALMAX:
            parser->logicalmax = data;

            break;

        case HID_GLOBAL_REPORTID:
            parser->reportid = data % REPORTIDS;
            mouse->usesids = 1;

            break;

        case HID_LOCAL_USAGE:
            if (parser->nusages < USAGES)
            {

                parser->usages[parser->nusages] = (size == 4) ? data : (parser->usagepage << 16) | data;
                parser->nusages++;

            }

            break;

        case HID_LOCAL_USAGEMIN:
            parser->usagemin = (size == 4) ? data : (parser->usagepage << 16) | data;

            break;

        case HID_LOCAL_USAGEMAX:
            parser->usagemax = (size == 4) ? data : (parser->usagepage << 16) | data;

            break;

        case HID_MAIN_INPUT:
            parseinput(parser, mouse, data);

            break;

        case HID_MAIN_FEATURE:
            parsepadfeature(parser, &hidpad);

            break;

        case HID_MAIN_COLLECTION:
            parser->depth++;

            if (parser->depth == 1)
                parser->application = getusage(parser, 0);

            if (parser->application == HID_USAGE_TOUCHPAD && !parser->fingerdepth && !parser->fingerdone && getusage(parser, 0) == HID_USAGE_FINGER)
                parser->fingerdepth = parser->depth;

            if (!parser->mousedepth && !mouse->x.found && data == HID_COLLECTION_APPLICATION && getusage(parser, 0) == HID_USAGE_MOUSE)
                parser->mousedepth = parser->depth;

            break;

        case HID_MAIN_ENDCOLLECTION:
            if (parser->depth == parser->mousedepth)
                parser->mousedepth = 0;

            if (parser->depth == parser->fingerdepth)
            {

                parser->fingerdepth = 0;
                parser->fingerdone = 1;

            }

            if (parser->depth == 1)
                parser->application = 0;

            if (parser->depth)
                parser->depth--;

            break;

        }

        if ((prefix & 0x0C) == 0x00)
        {

            parser->nusages = 0;
            parser->usagemin = 0;
            parser->usagemax = 0;

        }

        offset += 1 + size;

    }

    mouse->found = mouse->x.found && mouse->y.found && mouse->x.relative && mouse->y.relative;
    hidpad.found = hidpad.modereport && hidpad.tip.found && hidpad.x.found && hidpad.y.found && !hidpad.x.relative && hidpad.xmax && hidpad.ymax;
    mouse->length = 2 + mouse->usesids + (parser->offsets[mouse->reportid] + 7) / 8;

    /* reading just the mouse report keeps each poll short; the length word says what the device had */
    if (mouse->length > hid.inputlength || mouse->length > I2C_TRANSFER_IN)
        mouse->length = (hid.inputlength < I2C_TRANSFER_IN) ? hid.inputlength : I2C_TRANSFER_IN;

}

static void dumpreportdescriptor(struct i2c_transfer *transfer)
{

    unsigned int offset;

    for (offset = 0; offset < transfer->incount; offset += DUMPLINE)
    {

        char line[DUMPLINE * 3 + 1];
        unsigned int count = 0;
        unsigned int i;

        for (i = offset; i < offset + DUMPLINE && i < transfer->incount; i++)
        {

            count += cstring_write_value(line, DUMPLINE * 3, transfer->in[i], 16, 2, count);
            count += buffer_write(line, DUMPLINE * 3, " ", 1, count);

        }

        line[count] = '\0';

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c-hid: %u: %s", &offset, line);

    }

}

static int getbits(struct i2c_transfer *transfer, unsigned int start, struct field *field, unsigned int index)
{

    unsigned int value = 0;
    unsigned int i;

    for (i = 0; i < field->size; i++)
    {

        unsigned int bit = start * 8 + field->offset + index * field->size + i;

        if ((bit >> 3) < transfer->incount && (transfer->in[bit >> 3] & (1 << (bit & 7))))
            value |= 1 << i;

    }

    if (field->size > 1 && field->size < 32 && (value & (1 << (field->size - 1))))
        return (int)value - (1 << field->size);

    return value;

}

static char clamp(int value)
{

    return (value > 127) ? 127 : (value < -127) ? -127 : value;

}

static unsigned int ispadasserted(void)
{

    /* the line is active low; without the pad, read as before and let the device answer empty */
    return !pad.dw0 || gpio_islow(&pad);

}

static void notifymove(int x, int y)
{

    while (x || y)
    {

        char relx = clamp(x);
        char rely = clamp(y);

        mouse_notifymove(&mouseinterface, relx, rely);

        x -= relx;
        y -= rely;

    }

}

/* motion is sent at most once a tick: a task's mailbox holds 8 messages and kernel_notify drops what does not fit, so a stream of moves would push out presses and releases */
static void flushmotion(void)
{

    notifymove(motion.x, motion.y);

    if (motion.wheel)
        mouse_notifyscroll(&mouseinterface, -clamp(motion.wheel));

    motion.x = 0;
    motion.y = 0;
    motion.wheel = 0;

}

/* one report at a time, so a press and release that arrive in the same drain are both seen; motion before a button change is sent first to keep the order */
static void handlereport(struct i2c_transfer *transfer, unsigned int start)
{

    unsigned int buttons = 0;
    unsigned int i;

    for (i = 0; i < hidmouse.buttoncount; i++)
    {

        if (getbits(transfer, start, &hidmouse.buttons, i))
            buttons |= 1 << i;

    }

    if (buttons != hidmouse.oldbuttons)
        flushmotion();

    motion.x += getbits(transfer, start, &hidmouse.x, 0);
    motion.y += getbits(transfer, start, &hidmouse.y, 0);

    if (hidmouse.wheel.found && hidmouse.wheel.relative)
        motion.wheel += getbits(transfer, start, &hidmouse.wheel, 0);

    for (i = 0; i < hidmouse.buttoncount; i++)
    {

        unsigned int bit = 1 << i;

        if (!(hidmouse.oldbuttons & bit) && (buttons & bit))
            mouse_notifypress(&mouseinterface, i + 1);

        if ((hidmouse.oldbuttons & bit) && !(buttons & bit))
            mouse_notifyrelease(&mouseinterface, i + 1);

    }

    hidmouse.oldbuttons = buttons;

}

/* touchpad counts to pointer counts, the whole width being PADCOUNTS; the remainder is kept so slow movement is not lost */
static int scale(int delta, int *accumulator, unsigned int range, unsigned int counts)
{

    int quotient;

    *accumulator += delta * (int)counts;
    quotient = (*accumulator >= 0) ? *accumulator / (int)range : -(-*accumulator / (int)range);
    *accumulator -= quotient * (int)range;

    return quotient;

}

static void click(unsigned int button)
{

    flushmotion();
    mouse_notifypress(&mouseinterface, button);
    mouse_notifyrelease(&mouseinterface, button);

}

/* one finger moves the pointer, two scroll; a short touch that barely moves is a tap: one finger left click, two right click; the pad's own button is the left button */
static void handlepadreport(struct i2c_transfer *transfer, unsigned int start)
{

    unsigned int count = (hidpad.contactcount.found) ? getbits(transfer, start, &hidpad.contactcount, 0) : 0;
    unsigned int button = (hidpad.button.found) ? getbits(transfer, start, &hidpad.button, 0) & 1 : 0;
    unsigned int tip = getbits(transfer, start, &hidpad.tip, 0) & 1;
    unsigned int id = (hidpad.contactid.found) ? getbits(transfer, start, &hidpad.contactid, 0) & 0xFF : 0;
    int x = getbits(transfer, start, &hidpad.x, 0) & 0xFFFF;
    int y = getbits(transfer, start, &hidpad.y, 0) & 0xFFFF;

    if (button != touch.button)
    {

        flushmotion();

        if (button)
            mouse_notifypress(&mouseinterface, 1);
        else
            mouse_notifyrelease(&mouseinterface, 1);

        touch.button = button;

    }

    if (touch.active && count > touch.fingers)
        touch.fingers = count;

    if (!touch.active)
    {

        if (tip)
        {

            touch.active = 1;
            touch.id = id;
            touch.lastx = x;
            touch.lasty = y;
            touch.downtick = ticks;
            touch.travel = 0;
            touch.fingers = (count) ? count : 1;
            touch.accx = 0;
            touch.accy = 0;
            touch.accw = 0;

        }

        return;

    }

    if (id != touch.id)
        return;

    if (tip)
    {

        int dx = x - touch.lastx;
        int dy = y - touch.lasty;

        touch.lastx = x;
        touch.lasty = y;
        touch.travel += ((dx < 0) ? -dx : dx) + ((dy < 0) ? -dy : dy);

        if (touch.fingers >= 2)
            motion.wheel -= scale(dy, &touch.accw, hidpad.ymax, WHEELCOUNTS);

        else
        {

            motion.x += scale(dx, &touch.accx, hidpad.xmax, PADCOUNTS);
            motion.y += scale(dy, &touch.accy, hidpad.xmax, PADCOUNTS);

        }

        return;

    }

    if (ticks - touch.downtick <= TAPTICKS && touch.travel <= hidpad.xmax / 50 && !touch.button)
        click((touch.fingers >= 2) ? 2 : 1);

    touch.active = 0;

}

static unsigned int poll(unsigned int force)
{

    unsigned int start = (hidmouse.usesids) ? 3 : 2;
    unsigned int count = 0;
    unsigned int i;

    /* the touchpad drops a report nobody reads soon after its interrupt, so take all it has now; its line can already be released when the interrupt arrives, so an interrupt reads once regardless */
    for (i = 0; i < DRAINS && ((force && !i) || ispadasserted()); i++)
    {

        unsigned int tries;
        unsigned int ok = 0;

        /* the touchpad announced a report: a read that fails leaves it unread, and it waits for it before announcing more; a lost arbitration is retried at once, a timeout (already long) waits for the next tick */
        for (tries = 0; tries <= RETRIES && !ok; tries++)
        {

            ok = readinput(&transfer);

            if (!ok && !transfer.abort)
                break;

        }

        unread = !ok;

        if (!ok || getinword(&transfer, 0) < start + 1)
            break;

        if (padmode && transfer.in[2] == hidpad.reportid)
        {

            handlepadreport(&transfer, start);

            count++;

            continue;

        }

        if (padmode || (hidmouse.usesids && transfer.in[2] != hidmouse.reportid))
            continue;

        handlereport(&transfer, start);

        count++;

    }

    return count;

}

static void handleirq(unsigned int irq)
{

    spinlock_acquire(&spinlock);
    poll(unread);
    flushmotion();
    spinlock_release(&spinlock);

    ticks++;

}

static unsigned int handlepad(void)
{

    unsigned int found;

    spinlock_acquire(&spinlock);

    found = poll(1);

    spinlock_release(&spinlock);

    return found;

}

static void setupmouse(void)
{

    if (!sendcommand(&transfer, HID_POWERON) || !sendcommand(&transfer, HID_RESET))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c-hid: hid power on or reset failed, abort 0x%H8u", &transfer.abort);

        return;

    }

    i2c_wait(hid.id, RESETDELAY);

    if (hid.reportlength > I2C_TRANSFER_IN || !readreportdescriptor(&transfer))
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c-hid: report descriptor failed, abort 0x%H8u", &transfer.abort);

        return;

    }

    parsereportdescriptor(&transfer, &parser, &hidmouse);

    readlength = hidmouse.length;

    /* like Linux, put a Precision Touchpad into touchpad mode and do pointer motion and taps ourselves; its mouse emulation recognises taps poorly */
    if (hidpad.found && setfeature(&transfer, hidpad.modereport, HID_INPUTMODE_TOUCHPAD))
    {

        if (hidpad.selectreport)
            setfeature(&transfer, hidpad.selectreport, HID_SELECT_ALL);

        padmode = 1;
        readlength = (hid.inputlength < I2C_TRANSFER_IN) ? hid.inputlength : I2C_TRANSFER_IN;

        debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c-hid: touchpad mode, report %u, x max %u y max %u", &hidpad.reportid, &hidpad.xmax, &hidpad.ymax);

        return;

    }

    if (!hidmouse.found)
    {

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c-hid: no relative mouse in the report descriptor:");
        dumpreportdescriptor(&transfer);

        return;

    }

    debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c-hid: mouse report %u length %u buttons %u at bit %u x %u bits at %u y %u bits at %u wheel %u", &hidmouse.reportid, &hidmouse.length, &hidmouse.buttoncount, &hidmouse.buttons.offset, &hidmouse.x.size, &hidmouse.x.offset, &hidmouse.y.size, &hidmouse.y.offset, &hidmouse.wheel.found);

}

static void probe(unsigned int id, unsigned int known)
{

    unsigned int address = i2c_getaddress(id);
    unsigned int i;

    for (i = 0; i < HIDREGISTERS; i++)
    {

        unsigned int reg = (known) ? known : hidregisters[i];

        if (readhiddescriptor(&transfer, id, reg))
        {

            unsigned int vendor = getinword(&transfer, 20);
            unsigned int product = getinword(&transfer, 22);

            hid.id = id;
            hid.reportlength = getinword(&transfer, 4);
            hid.reportregister = getinword(&transfer, 6);
            hid.inputregister = getinword(&transfer, 8);
            hid.inputlength = getinword(&transfer, 10);
            hid.commandregister = getinword(&transfer, 16);
            hid.dataregister = getinword(&transfer, 18);

            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c-hid: hid 0x%H2u descriptor at 0x%H4u vendor 0x%H4u product 0x%H4u", &address, &reg, &vendor, &product);
            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c-hid: report descriptor %u bytes at 0x%H4u input 0x%H4u max %u command 0x%H4u", &hid.reportlength, &hid.reportregister, &hid.inputregister, &hid.inputlength, &hid.commandregister);
            setupmouse();

            return;

        }

        debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c-hid: hid 0x%H2u descriptor at 0x%H4u failed, abort 0x%H8u", &address, &reg, &transfer.abort);

        if (known)
            return;

    }

}

static void driver_init(unsigned int id)
{

}

static unsigned int driver_match(unsigned int type, unsigned int id)
{

    return type == I2C_BUS && i2c_ishid(id);

}

static void driver_reset(unsigned int id)
{

    probe(id, i2c_gethidregister(id));

}

static void driver_attach(unsigned int id)
{

    if (hidmouse.found || padmode)
    {

        struct i2c_pad interrupt = i2c_getpad(id);

        mouse_registerinterface(&mouseinterface);
        pic_setroutine(TIMERIRQ, handleirq);
        apic_setroutine(TIMERIRQ, handleirq);

        if (interrupt.found)
            pad = gpio_findpad(interrupt.group, interrupt.pad);

        if (pad.dw0)
            padinterrupt = gpio_setinterrupt(&pad, handlepad);

        if (padinterrupt)
            debug_fmt(DEBUG_INFO, __FILE__, __LINE__, "i2c-hid: touchpad interrupt on");
        else
            debug_fmt(DEBUG_WARNING, __FILE__, __LINE__, "i2c-hid: no touchpad interrupt, polling");

    }

}

static void driver_detach(unsigned int id)
{

    if (hidmouse.found || padmode)
    {

        if (padinterrupt)
            gpio_unsetinterrupt(&pad);

        padinterrupt = 0;

        pic_unsetroutine(TIMERIRQ, handleirq);
        apic_unsetroutine(TIMERIRQ, handleirq);
        mouse_unregisterinterface(&mouseinterface);

    }

}

void module_init(void)
{

    mouse_initinterface(&mouseinterface);
    spinlock_init(&spinlock);
    base_initdriver(&driver, "i2c-hid", driver_init, driver_match, driver_reset, driver_attach, driver_detach);

}

void module_register(void)
{

    base_registerdriver(&driver);

}

void module_unregister(void)
{

    base_unregisterdriver(&driver);

}

