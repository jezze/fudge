struct descriptor
{

    struct service *service;
    unsigned int id;

};

unsigned int descriptor_check(struct descriptor *descriptor);
void descriptor_reset(struct descriptor *descriptor);
void descriptor_init(struct descriptor *descriptor);
