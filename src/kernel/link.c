#include <fudge.h>
#include "link.h"

void link_init(struct link *link)
{

    list_inititem(&link->item, link);

}

