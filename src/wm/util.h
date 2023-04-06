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

int util_intersects(int x, int min, int max);
int util_min(int a, int b);
int util_max(int a, int b);
int util_clamp(int v, int min, int max);
void util_initposition(struct util_position *position, int x, int y);
void util_initsize(struct util_size *size, int w, int h);
