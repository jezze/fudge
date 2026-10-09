struct base_driver
{

    struct resource resource;
    char *name;
    unsigned int (*match)(unsigned int id);
    void (*init)(unsigned int id);
    void (*reset)(unsigned int id);
    void (*attach)(unsigned int id);
    void (*detach)(unsigned int id);

};

void base_registerdriver(struct base_driver *driver, unsigned int type);
void base_unregisterdriver(struct base_driver *driver, unsigned int type);
void base_initdriver(struct base_driver *driver, char *name, void (*init)(unsigned int id), unsigned int (*match)(unsigned int id), void (*reset)(unsigned int id), void (*attach)(unsigned int id), void (*detach)(unsigned int id));
