struct channel
{

    unsigned int poll;
    unsigned int source;
    struct {struct ipc_header header; char data[FUDGE_BSIZE];} message;
    struct {void (*callback)(struct channel *channel, void *data, unsigned int size); unsigned int redirect;} signals[EVENTS];

};

unsigned int channel_place(struct channel *channel, unsigned int id);
unsigned int channel_write(struct channel *channel, unsigned int descriptor);
void channel_listen(struct channel *channel);
void channel_close(struct channel *channel);
void channel_setredirect(struct channel *channel, unsigned int type, unsigned int id);
void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, void *mdata, unsigned int msize));
void channel_request(struct channel *channel, unsigned int type);
unsigned int channel_reply(struct channel *channel, unsigned int type);
void channel_append(struct channel *channel, unsigned int count, void *buffer);
void channel_appendstring(struct channel *channel, char *string);
void channel_appendvalue(struct channel *channel, int value, unsigned int base, unsigned int padding);
void channel_init(struct channel *channel);
