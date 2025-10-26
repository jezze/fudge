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

    int x;
    int y;
    unsigned int w;
    unsigned int h;

};

struct util_token
{

    unsigned int key;
    char *value;

};

unsigned int util_intersects(int x, int min, int max);
unsigned int util_intersects_region(struct util_region *region, int x, int y);
int util_min(int a, int b);
int util_max(int a, int b);
int util_clamp(int v, int min, int max);
struct util_position util_position(int x, int y);
struct util_size util_size(int w, int h);
struct util_region util_region(int x, int y, unsigned int w, unsigned int h);
unsigned int util_getkey(struct util_token *tokens, unsigned int n, char *value);
