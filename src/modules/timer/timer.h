struct timer_interface
{

    struct resource resource;
    unsigned int id;
    unsigned int ichannel1;
    unsigned int ichannel10;
    unsigned int ichannel100;
    unsigned int ichannel1000;

};

void timer_notifytick1(struct timer_interface *interface, unsigned int counter);
void timer_notifytick10(struct timer_interface *interface, unsigned int counter);
void timer_notifytick100(struct timer_interface *interface, unsigned int counter);
void timer_notifytick1000(struct timer_interface *interface, unsigned int counter);
void timer_registerinterface(struct timer_interface *interface);
void timer_unregisterinterface(struct timer_interface *interface);
void timer_initinterface(struct timer_interface *interface, unsigned int id, unsigned int ichannel1, unsigned int ichannel10, unsigned int ichannel100, unsigned int ichannel1000);
