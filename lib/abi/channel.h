struct channel
{

    unsigned int poll;
    struct {void (*callback)(struct channel *channel, unsigned int source, void *data, unsigned int size); unsigned int redirect;} signals[EVENTS];

};

void channel_dispatch(struct channel *channel, struct message_header *header, struct message_data *data);
unsigned int channel_place(struct channel *channel, unsigned int id, unsigned int type, unsigned int count, void *data);
unsigned int channel_poll(struct channel *channel, struct message_header *header, struct message_data *data);
unsigned int channel_pollonly(struct channel *channel, unsigned int source, unsigned int type, struct message_header *header, struct message_data *data);
unsigned int channel_polltype(struct channel *channel, unsigned int type, struct message_header *header, struct message_data *data);
unsigned int channel_pollsource(struct channel *channel, unsigned int source, struct message_header *header, struct message_data *data);
void channel_close(struct channel *channel);
void channel_setredirect(struct channel *channel, unsigned int type, unsigned int mode, unsigned int id, unsigned int source);
void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, unsigned int source, void *mdata, unsigned int msize));
void channel_init(struct channel *channel);
