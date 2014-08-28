#include <module.h>
#include <kernel/resource.h>
#include <kernel/vfs.h>
#include <system/system.h>
#include <base/base.h>

static struct system_node root;

void init()
{

    system_init_stream(&root, "null");
    system_register_node(&root);

}

void destroy()
{

    system_unregister_node(&root);

}

