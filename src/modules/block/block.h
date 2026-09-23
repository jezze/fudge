#define BLOCK_TYPE_READ                 1
#define BLOCK_TYPE_WRITE                2

struct block_session
{

    unsigned int type;
    unsigned int source;
    unsigned int start;
    unsigned int count;
    unsigned int offset;

};

struct block_interface
{

    struct resource resource;
    struct service service;
    unsigned int id;
    unsigned int inode;
    struct block_session sessions[8];
    void (*oninfo)(struct event_blockinfo *blockinfo);
    void (*startsession)(struct block_session *session);

};

struct block_session *block_getsession(struct block_interface *interface);
void block_session_done(struct block_interface *interface, struct block_session *session);
void block_registerinterface(struct block_interface *interface);
void block_unregisterinterface(struct block_interface *interface);
void block_initinterface(struct block_interface *interface, unsigned int id, void (*oninfo)(struct event_blockinfo *blockinfo), void (*startsession)(struct block_session *session));
