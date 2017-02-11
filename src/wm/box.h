struct box
{

    unsigned int x;
    unsigned int y;
    unsigned int w;
    unsigned int h;

};

unsigned int box_isinside(struct box *box, unsigned int x, unsigned int y);
void box_setsize(struct box *box, unsigned int x, unsigned int y, unsigned int w, unsigned int h);
void box_resize(struct box *box, int f);
