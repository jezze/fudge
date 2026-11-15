struct power_interface
{

    struct resource resource;
    unsigned int (*poweroff)(void);
    unsigned int (*reboot)(void);

};

void power_registerinterface(struct power_interface *interface);
void power_unregisterinterface(struct power_interface *interface);
void power_initinterface(struct power_interface *interface, unsigned int (*poweroff)(void), unsigned int (*reboot)(void));
