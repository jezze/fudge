#define POOL_TASKS                      64
#define POOL_NODES                      1024
#define POOL_MAILBOXES                  512

struct node *pool_getnode(unsigned int inode);
unsigned int pool_getinodefromitem(struct list_item *item);
struct task *pool_gettask(unsigned int itask);
struct list_item *pool_gettaskitem(unsigned int itask);
unsigned int pool_getitaskfromitem(struct list_item *item);
struct mailbox *pool_getmailbox(unsigned int imailbox);
unsigned int pool_addnodeto(struct list *nodes, char *name, struct resource *resource, struct node_operands *operands);
unsigned int pool_addnode(char *name, struct resource *resource, struct node_operands *operands);
void pool_removenodefrom(struct list *nodes, unsigned int inode);
unsigned int pool_addmailbox(unsigned int itask);
void pool_removemailboxes(unsigned int itask);
unsigned int pool_findinode(unsigned int namehash, unsigned int index);
unsigned int pool_createtask(void);
void pool_destroytask(unsigned int itask);
void pool_setup(unsigned int mbaddress, unsigned int mbsize);
