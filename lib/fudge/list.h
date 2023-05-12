struct list_item
{

    struct list_item *next;
    struct list_item *prev;
    void *data;

};

struct list
{

    struct spinlock spinlock;
    struct list_item *head;
    struct list_item *tail;
    unsigned int count;

};

void list_add_unsafe(struct list *list, struct list_item *item);
void list_add(struct list *list, struct list_item *item);
void list_remove_unsafe(struct list *list, struct list_item *item);
void list_remove(struct list *list, struct list_item *item);
void list_move_unsafe(struct list *to, struct list *from, struct list_item *item);
void list_move(struct list *to, struct list *from, struct list_item *item);
struct list_item *list_pickhead(struct list *list);
struct list_item *list_picktail(struct list *list);
void list_inititem(struct list_item *item, void *data);
void list_init(struct list *list);
