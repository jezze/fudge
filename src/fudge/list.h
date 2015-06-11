struct list_item
{

    struct list_item *next;
    struct list_item *prev;
    void *data;

};

struct list
{

    struct list_item *head;
    struct list_item *tail;

};

void list_add(struct list *list, struct list_item *item);
void list_remove(struct list *list, struct list_item *item);
void list_move(struct list *out, struct list *in, struct list_item *item);
unsigned int list_count(struct list *list);
void list_inititem(struct list_item *item, void *data);
void list_init(struct list *list);
