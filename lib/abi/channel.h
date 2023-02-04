#define CHANNEL_DEFAULT                 0

void channel_dispatch(struct message *message, void *data);
unsigned int channel_send(unsigned int target, unsigned int event);
unsigned int channel_sendbuffer(unsigned int target, unsigned int event, unsigned int count, void *data);
unsigned int channel_sendfmt0(unsigned int target, unsigned int event, char *fmt);
unsigned int channel_sendfmt1(unsigned int target, unsigned int event, char *fmt, void *arg1);
unsigned int channel_sendfmt2(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2);
unsigned int channel_sendfmt3(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3);
unsigned int channel_sendfmt4(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4);
unsigned int channel_sendfmt6(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6);
unsigned int channel_sendfmt8(unsigned int target, unsigned int event, char *fmt, void *arg1, void *arg2, void *arg3, void *arg4, void *arg5, void *arg6, void *arg7, void *arg8);
unsigned int channel_listen(unsigned int target, unsigned int event);
unsigned int channel_forward(unsigned int target, unsigned int event, unsigned int id);
unsigned int channel_pick(struct message *message, unsigned int count, void *data);
unsigned int channel_process(void);
unsigned int channel_poll(unsigned int event, struct message *message, unsigned int count, void *data);
unsigned int channel_pollfrom(unsigned int source, unsigned int event, struct message *message, unsigned int count, void *data);
unsigned int channel_read(unsigned int event, unsigned int count, void *data);
unsigned int channel_readfrom(unsigned int source, unsigned int event, unsigned int count, void *data);
unsigned int channel_wait(unsigned int source, unsigned int event);
void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize));
void channel_route(unsigned int event, unsigned int mode, unsigned int id, unsigned int source);
void channel_open(void);
void channel_close(void);
void channel_error(char *description);
void channel_warning(char *description);
