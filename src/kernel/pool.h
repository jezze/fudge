#define POOL_CORES                      256
#define POOL_TASKS                      32
#define POOL_NODES                      512
#define POOL_MAILBOXES                  64

struct core *pool_getcore(unsigned int icore);
struct mailbox *pool_getmailbox(unsigned int imailbox);
struct resource *pool_getnoderesource(unsigned int inode);
struct task *pool_gettask(unsigned int itask);
unsigned int pool_getinodefromitem(struct list_item *item);
unsigned int pool_getitaskfromitem(struct list_item *item);
unsigned int pool_pickcorefrom(struct list *list);
unsigned int pool_pickmailbox(void);
unsigned int pool_picknode(struct resource *resource);
unsigned int pool_picktask(void);
unsigned int pool_picktaskfrom(struct list *list);
void pool_unpickmailbox(unsigned int imailbox);
void pool_unpicknode(unsigned int inode);
unsigned int pool_renewnode(unsigned int inode);
void pool_unpicktask(unsigned int itask);
void pool_placecore(unsigned int icore, struct list *list);
void pool_placenode(unsigned int inode, struct list *list);
void pool_placetask(unsigned int itask, struct list *list);
unsigned int pool_attachnode(struct resource *resource);
void pool_detachnode(struct resource *resource);
void pool_setroot(struct resource *resource);
void pool_setup(unsigned long mbaddress);
