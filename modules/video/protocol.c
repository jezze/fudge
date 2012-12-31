#include <fudge/memory.h>
#include <base/base.h>
#include "video.h"

void video_init_protocol(struct video_protocol *protocol, char *name)
{

    memory_clear(protocol, sizeof (struct video_protocol));

    protocol->name = name;

}

