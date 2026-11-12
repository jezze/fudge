#define POOL_CORES                      256
#define POOL_TASKMAILBOXES              2
#define POOL_TASKMMUSIZE                0x20000
#define POOL_PERTASK                    (TASK_CODESIZE + TASK_STACKSIZE + TASK_BUFFERSIZE + MMAP_SIZE + POOL_TASKMMUSIZE + MESSAGE_CAPACITY * POOL_TASKMAILBOXES)
#define POOL_CHUNK                      0x04880000
#define POOL_TASKS                      (POOL_CHUNK / POOL_PERTASK)
#define POOL_MAILBOXES                  (POOL_TASKS * POOL_TASKMAILBOXES)
#define POOL_TASKNODES                  8
#define POOL_NODES                      (256 + POOL_TASKS * POOL_TASKNODES)
#define POOL_LINKS                      (POOL_TASKS * POOL_TASKNODES)

struct core *pool_getcore(unsigned int icore);
struct mailbox *pool_getmailbox(unsigned int imailbox);
struct resource *pool_getnoderesource(unsigned int inode);
struct task *pool_gettask(unsigned int itask);
unsigned int pool_getlinktarget(struct list_item *item);
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
void pool_placetask(unsigned int itask, struct list *list);
unsigned int pool_link(struct list *links, unsigned int target);
void pool_unlink(struct list_item *item);
unsigned int pool_attachnode(struct resource *resource);
void pool_detachnode(struct resource *resource);
void pool_setroot(struct resource *resource);
void pool_setup(unsigned long mbaddress);
