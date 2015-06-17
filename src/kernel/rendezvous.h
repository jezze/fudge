void rendezvous_attach(struct list *mailboxes);
void rendezvous_detach(struct list *mailboxes);
void rendezvous_detachlist(struct list *mailboxes);
unsigned int rendezvous_read(unsigned int size, unsigned int count, void *buffer);
unsigned int rendezvous_write(unsigned int id, unsigned int size, unsigned int count, void *buffer);
void rendezvous_writelist(struct list *mailboxes, unsigned int size, unsigned int count, void *buffer);

