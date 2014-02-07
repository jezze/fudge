struct rendezvous
{

    struct task *task;

};

void rendezvous_sleep(struct rendezvous *rendezvous, unsigned int condition);
void rendezvous_unsleep(struct rendezvous *rendezvous, unsigned int condition);
void rendezvous_init();
