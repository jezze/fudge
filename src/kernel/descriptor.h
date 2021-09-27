struct descriptor
{

    struct service *service;
    unsigned int id;
    unsigned int offset;
    unsigned int current;
    unsigned int count;

};

unsigned int descriptor_check(struct descriptor *descriptor);
