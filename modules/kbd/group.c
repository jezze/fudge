#include <fudge/memory.h>
#include <system/system.h>
#include "kbd.h"

void kbd_init_group(struct kbd_group *group)
{

    memory_clear(group, sizeof (struct system_group));

    system_init_group(&group->base, "kbd");

}

