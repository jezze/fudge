#include <fudge/memory.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include "video.h"
#include "group.h"

void video_init_group(struct video_group *group)
{

    memory_clear(group, sizeof (struct video_group));

    system_init_group(&group->base, "video");

}

