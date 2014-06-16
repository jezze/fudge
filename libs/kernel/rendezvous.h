struct rendezvous
{

    struct task *task;
    unsigned int sleep;

};

unsigned int rendezvous_islocked(struct rendezvous *rendezvous);
unsigned int rendezvous_lock(struct rendezvous *rendezvous);
unsigned int rendezvous_unlock(struct rendezvous *rendezvous);
unsigned int rendezvous_sleep(struct rendezvous *rendezvous, unsigned int condition);
unsigned int rendezvous_unsleep(struct rendezvous *rendezvous, unsigned int condition);
void rendezvous_setup();
