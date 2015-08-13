struct client
{

    struct list_item item;
    struct window window;
    unsigned int source;

};

void client_init(struct client *client);
