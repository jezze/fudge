unsigned int rendezvous_read(struct task *task, unsigned int size, unsigned int count, void *buffer);
unsigned int rendezvous_write(struct task *task, unsigned int size, unsigned int count, void *buffer);
void rendezvous_writelist(struct list *mailboxes, unsigned int size, unsigned int count, void *buffer);
