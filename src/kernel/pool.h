#define POOL_CORES                      256
#define POOL_TASKS                      64
#define POOL_NODES                      1024
#define POOL_MAILBOXES                  512

struct core *pool_getcore(unsigned int icore);
struct mailbox *pool_getmailbox(unsigned int imailbox);
struct node *pool_getnode(unsigned int inode);
struct task *pool_gettask(unsigned int itask);
unsigned int pool_getinodefromitem(struct list_item *item);
unsigned int pool_getitaskfromitem(struct list_item *item);
unsigned int pool_pickcorefrom(struct list *list);
unsigned int pool_pickmailbox(void);
unsigned int pool_picknode(void);
unsigned int pool_picktask(void);
unsigned int pool_picktaskfrom(struct list *list);
void pool_unpickmailbox(unsigned int imailbox);
void pool_unpicknode(unsigned int inode);
void pool_unpicktask(unsigned int itask);
void pool_placecore(unsigned int icore, struct list *list);
void pool_placenode(unsigned int inode, struct list *list);
void pool_placetask(unsigned int itask, struct list *list);
unsigned int pool_findinode(unsigned int namehash, unsigned int index);
void pool_setup(unsigned long saddress, unsigned int ssize, unsigned long mbaddress, unsigned int mbsize);
