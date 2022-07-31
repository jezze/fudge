struct list_item *pool_next(struct list_item *current);
struct list_item *pool_nextin(struct list_item *current, struct widget *parent);
struct widget *pool_getwidgetbyid(unsigned int source, char *id);
void pool_bump(struct widget *widget);
struct widget *pool_create(unsigned int source, unsigned int type, char *id, char *in);
char *pool_savestring(char *string);
char *pool_freestring(char *string);
char *pool_replacestring(char *current, char *string);
void pool_setup(void);
