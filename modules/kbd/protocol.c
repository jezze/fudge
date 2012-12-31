#include <fudge/memory.h>
#include <base/base.h>
#include "kbd.h"

void kbd_init_protocol(struct kbd_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct kbd_protocol));

    protocol->name = name;

}

