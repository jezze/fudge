struct battery_interface
{

    struct resource resource;
    unsigned int unit;
    unsigned int (*getpresent)(void);
    unsigned int (*getstate)(void);
    unsigned int (*getrate)(void);
    unsigned int (*getremaining)(void);
    unsigned int (*getfull)(void);
    unsigned int (*getdesign)(void);
    unsigned int (*getvoltage)(void);

};

void battery_registerinterface(struct battery_interface *interface);
void battery_unregisterinterface(struct battery_interface *interface);
void battery_initinterface(struct battery_interface *interface, unsigned int unit, unsigned int (*getpresent)(void), unsigned int (*getstate)(void), unsigned int (*getrate)(void), unsigned int (*getremaining)(void), unsigned int (*getfull)(void), unsigned int (*getdesign)(void), unsigned int (*getvoltage)(void));
