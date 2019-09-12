union channel_message
{

    struct event_header header;
    char buffer[FUDGE_BSIZE];

};

struct channel
{

    unsigned int state;
    void (*signals[EVENTS])(struct channel *channel, void *mdata, unsigned int msize);
    struct event_header i;
    union channel_message o;

};

void channel_place(struct channel *channel, unsigned int id);
void channel_listen(struct channel *channel);
void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, void *mdata, unsigned int msize));
void channel_forward(struct channel *channel, unsigned int type);
void channel_request(struct channel *channel, unsigned int type);
void channel_request2(struct channel *channel, unsigned int type, unsigned int session);
unsigned int channel_reply(struct channel *channel, unsigned int type);
void channel_append(struct channel *channel, unsigned int count, void *buffer);
void channel_init(struct channel *channel);
