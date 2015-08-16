struct client
{

    struct list_item item;
    struct window window;
    unsigned int source;

};

void client_activate(struct client *client);
void client_deactivate(struct client *client);
void client_init(struct client *client);
