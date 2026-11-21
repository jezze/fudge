#define KEYBOARD_RING                   64
#define KEYBOARD_SUBSCRIBERS            8

struct keyboard_entry
{

    unsigned int event;
    unsigned char scancode;

};

struct keyboard_subscriber
{

    unsigned int target;
    unsigned int bit;
    unsigned int next;

};

struct keyboard_interface
{

    struct resource resource;
    struct spinlock spinlock;
    struct keyboard_entry ring[KEYBOARD_RING];
    unsigned int head;
    struct keyboard_subscriber subscribers[KEYBOARD_SUBSCRIBERS];

};

void keyboard_notifypress(struct keyboard_interface *interface, unsigned char scancode);
void keyboard_notifyrelease(struct keyboard_interface *interface, unsigned char scancode);
void keyboard_registerinterface(struct keyboard_interface *interface);
void keyboard_unregisterinterface(struct keyboard_interface *interface);
void keyboard_initinterface(struct keyboard_interface *interface);
