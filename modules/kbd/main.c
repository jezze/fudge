#include <system/system.h>
#include "kbd.h"

static struct kbd_group group;

void kbd_register_interface(struct kbd_interface *interface)
{

}

void kbd_register_protocol(struct kbd_protocol *protocol)
{

}

void kbd_unregister_interface(struct kbd_interface *interface)
{

}

void kbd_unregister_protocol(struct kbd_protocol *protocol)
{

}

void init()
{

    kbd_init_group(&group);
    system_register_group(&group.base);

}

void destroy()
{

    system_unregister_group(&group.base);

}

