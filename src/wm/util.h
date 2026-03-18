struct util_position
{

    int x;
    int y;

};

struct util_size
{

    unsigned int w;
    unsigned int h;

};

struct util_region
{

    struct util_position position;
    struct util_size size;

};

struct util_token
{

    unsigned int key;
    char *value;

};

unsigned int util_intersects(int x, int min, int max);
int util_min(int a, int b);
int util_max(int a, int b);
int util_clamp(int v, int min, int max);
struct util_position util_position(int x, int y);
struct util_size util_size(int w, int h);
struct util_size util_size_intersection(struct util_size *a, struct util_size *b);
struct util_region util_region(int x, int y, unsigned int w, unsigned int h);
unsigned int util_region_intersects(struct util_region *region, int x, int y);
struct util_region util_region_intersection(struct util_region *a, struct util_region *b);
unsigned int util_getkey(struct util_token *tokens, unsigned int n, char *value);
