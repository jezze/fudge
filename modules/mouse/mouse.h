struct mouse_interface
{

    struct base_driver *driver;
    char vx;
    char vy;

};

struct mouse_interface_node
{

    struct system_group root;
    struct system_integer vx;
    struct system_integer vy;
    struct mouse_interface *interface;

};

struct mouse_protocol
{

    char *name;

};

struct mouse_protocol_node
{

    struct system_group root;
    struct mouse_protocol *protocol;

};

void mouse_register_interface(struct mouse_interface *interface);
void mouse_register_protocol(struct mouse_protocol *protocol);
void mouse_unregister_interface(struct mouse_interface *interface);
void mouse_unregister_protocol(struct mouse_protocol *protocol);
void mouse_init_interface(struct mouse_interface *interface, struct base_driver *driver);
void mouse_init_protocol(struct mouse_protocol *protocol, char *name);
