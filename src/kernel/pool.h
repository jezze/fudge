#define POOL_CORES                      256
#define POOL_TASKS                      64
#define POOL_NODES                      1024
#define POOL_MAILBOXES                  512

struct core *pool_getcore(unsigned int icore);
struct list_item *pool_getcoreitem(unsigned int icore);
struct mailbox *pool_getmailbox(unsigned int imailbox);
struct list_item *pool_getmailboxitem(unsigned int imailbox);
struct node *pool_getnode(unsigned int inode);
struct list_item *pool_getnodeitem(unsigned int inode);
struct task *pool_gettask(unsigned int itask);
struct list_item *pool_gettaskitem(unsigned int itask);
unsigned int pool_getinodefromitem(struct list_item *item);
unsigned int pool_getitaskfromitem(struct list_item *item);
unsigned int pool_pickmailbox(unsigned int itask);
unsigned int pool_picknode(char *name, struct resource *resource, struct node_operands *operands);
unsigned int pool_picktask(void);
void pool_unpickmailbox(unsigned int imailbox);
void pool_unpicktask(unsigned int itask);
void pool_addnode(struct list *nodes, unsigned int inode);
void pool_removenode(struct list *nodes, unsigned int inode);
unsigned int pool_findinode(unsigned int namehash, unsigned int index);
void pool_setup(unsigned long saddress, unsigned int ssize, unsigned long mbaddress, unsigned int mbsize);
