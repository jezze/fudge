struct list_item
{

    struct list *list;
    struct list_item *next;
    struct list_item *prev;
    void *data;

};

struct list
{

    struct list_item *head;
    struct list_item *tail;
    unsigned int count;

};

void list_add(struct list *list, struct list_item *item);
void list_remove(struct list *list, struct list_item *item);
void list_move(struct list *list, struct list_item *item);
unsigned int list_find(struct list *list, struct list_item *item);
void list_inititem(struct list_item *item, void *data);
