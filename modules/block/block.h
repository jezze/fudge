struct block_protocol
{

    char *name;

};

void block_register_protocol(struct block_protocol *protocol);
void block_unregister_protocol(struct block_protocol *protocol);
void block_init_protocol(struct block_protocol *protocol, char *name);
