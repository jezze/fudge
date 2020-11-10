struct base_bus
{

    struct resource resource;
    unsigned int type;
    char *name;
    void (*setup)(void);
    unsigned int (*next)(unsigned int id);

};

void base_registerbus(struct base_bus *bus);
void base_unregisterbus(struct base_bus *bus);
void base_initbus(struct base_bus *bus, unsigned int type, char *name, void (*setup)(void), unsigned int (*next)(unsigned int id));
