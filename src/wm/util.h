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

struct util_box
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

int util_intersects(int x, int min, int max);
int util_min(int a, int b);
int util_max(int a, int b);
int util_clamp(int v, int min, int max);
void util_initposition(struct util_position *position, int x, int y);
void util_initsize(struct util_size *size, int w, int h);
void util_initbox(struct util_box *box, int x, int y, unsigned int w, unsigned int h);
unsigned int util_getkey(struct util_token *tokens, unsigned int n, char *value);
