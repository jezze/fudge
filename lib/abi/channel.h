#define CHANNEL_CALLBACKS               256

struct channel;

struct channel_callback
{

    void (*callback)(struct channel *channel, unsigned int source, void *data, unsigned int size);
    unsigned int redirect;

};

struct channel
{

    unsigned int poll;
    struct channel_callback *callbacks;

};

void channel_dispatch(struct channel *channel, struct message_header *header, struct message_data *data);
unsigned int channel_place(struct channel *channel, unsigned int id, unsigned int event, unsigned int count, void *data);
unsigned int channel_poll(struct channel *channel, struct message_header *header, struct message_data *data);
unsigned int channel_pollevent(struct channel *channel, unsigned int event, struct message_header *header, struct message_data *data);
unsigned int channel_polldescriptor(struct channel *channel, unsigned int descriptor, struct message_header *header, struct message_data *data);
unsigned int channel_pollsource(struct channel *channel, unsigned int source, struct message_header *header, struct message_data *data);
unsigned int channel_pollsourceevent(struct channel *channel, unsigned int source, unsigned int event, struct message_header *header, struct message_data *data);
void channel_close(struct channel *channel, unsigned int source);
void channel_setredirect(struct channel *channel, unsigned int event, unsigned int mode, unsigned int id, unsigned int source);
void channel_setcallback(struct channel *channel, unsigned int event, void (*callback)(struct channel *channel, unsigned int source, void *mdata, unsigned int msize));
void channel_init(struct channel *channel, struct channel_callback *callbacks);
