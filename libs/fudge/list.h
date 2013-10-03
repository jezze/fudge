struct list_item
{

    struct list_item *next;
    void *self;

};

struct list
{

    struct list_item *head;

};

void list_add(struct list *list, struct list_item *item);
void list_remove(struct list *list, struct list_item *item);
void list_init_item(struct list_item *item, void *self);
void list_init(struct list *list);
