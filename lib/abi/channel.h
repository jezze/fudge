struct channel
{

    unsigned int poll;
    union {struct ipc_header header; char data[FUDGE_BSIZE];} message;
    struct {void (*callback)(struct channel *channel, unsigned int source, void *data, unsigned int size); unsigned int redirect;} signals[EVENTS];

};

unsigned int channel_place(struct channel *channel, unsigned int id);
unsigned int channel_write(struct channel *channel, unsigned int descriptor);
unsigned int channel_poll(struct channel *channel, struct ipc_header *header, void *data, unsigned int type);
void channel_listen(struct channel *channel);
void channel_listen2(struct channel *channel, void (*oninit)(struct channel *channel), void (*onexit)(struct channel *channel));
void channel_close(struct channel *channel);
void channel_setredirect(struct channel *channel, unsigned int type, unsigned int mode, unsigned int id, unsigned int source);
void channel_setsignal(struct channel *channel, unsigned int type, void (*callback)(struct channel *channel, unsigned int source, void *mdata, unsigned int msize));
void channel_request(struct channel *channel, unsigned int type);
void channel_append(struct channel *channel, unsigned int count, void *buffer);
void channel_appendstring(struct channel *channel, char *string);
void channel_appendstring2(struct channel *channel, char *string);
void channel_appendvalue(struct channel *channel, int value, unsigned int base, unsigned int padding);
void channel_init(struct channel *channel);
void channel_sendfile(struct channel *channel, unsigned int id, char *path);
void channel_sendmain(struct channel *channel, unsigned int id);
void channel_sendredirect(struct channel *channel, unsigned int id, unsigned int type, unsigned int mode, unsigned int source);
