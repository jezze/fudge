void channel_dispatch(struct message *message);
unsigned int channel_send(unsigned int event);
unsigned int channel_sendto(unsigned int target, unsigned int event);
unsigned int channel_sendbuffer(unsigned int event, unsigned int count, void *data);
unsigned int channel_sendbufferto(unsigned int target, unsigned int event, unsigned int count, void *data);
unsigned int channel_sendstring(unsigned int event, char *string);
unsigned int channel_sendstringto(unsigned int target, unsigned int event, char *string);
unsigned int channel_sendstringz(unsigned int event, char *string);
unsigned int channel_sendstringzto(unsigned int target, unsigned int event, char *string);
unsigned int channel_sendmessage(struct message *message);
unsigned int channel_sendmessageto(unsigned int target, struct message *message);
unsigned int channel_redirecttarget(unsigned int target, unsigned int event, unsigned int id);
unsigned int channel_redirectback(unsigned int target, unsigned int event);
unsigned int channel_pick(struct message *message);
unsigned int channel_process(void);
unsigned int channel_pollfrom(unsigned int source, struct message *message);
unsigned int channel_pollevent(unsigned int event, struct message *message);
unsigned int channel_polleventfrom(unsigned int source, unsigned int event, struct message *message);
unsigned int channel_kpoll(struct message *message);
unsigned int channel_kpollevent(unsigned int event, struct message *message);
unsigned int channel_readmessage(struct message *message);
unsigned int channel_readmessagefrom(unsigned int source, struct message *message);
unsigned int channel_wait(unsigned int source, unsigned int event);
void channel_bind(unsigned int event, void (*callback)(unsigned int source, void *mdata, unsigned int msize));
void channel_route(unsigned int event, unsigned int mode, unsigned int id, unsigned int source);
void channel_open(void);
void channel_close(void);
void channel_error(char *description);
void channel_warning(char *description);
