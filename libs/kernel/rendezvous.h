struct rendezvous
{

    struct task *task;

};

void rendezvous_lock(struct rendezvous *rendezvous);
void rendezvous_unlock(struct rendezvous *rendezvous);
void rendezvous_sleep(struct rendezvous *rendezvous, unsigned int condition);
void rendezvous_unsleep(struct rendezvous *rendezvous, unsigned int condition);
void rendezvous_setup();
