struct channel
{

    unsigned int state;
    void (*signals[EVENTS])(struct channel *channel);
    union event_message i;
    union event_message o;

};

void *channel_getdata(struct channel *channel);
unsigned int channel_getdatasize(struct channel *channel);
unsigned int channel_pick(struct channel *channel);
unsigned int channel_place(unsigned int id, union event_message *message);
void channel_dispatch(struct channel *channel, unsigned int type);
unsigned int channel_listen(struct channel *channel);
void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel));
void channel_forward(struct channel *channel, unsigned int type);
void channel_request(struct channel *channel, unsigned int type, unsigned int session);
void channel_reply(struct channel *channel, unsigned int type);
void channel_init(struct channel *channel);
