struct scene
{

    unsigned int used;
    unsigned int framestart;
    unsigned int framestop;
    unsigned int duration;
    void (*setup)(struct scene *scene);
    void (*render)(struct scene *scene, unsigned int frame, unsigned int loopframe, unsigned int globalframe);

};

struct vector2
{

    double x;
    double y;

};

struct vector3
{

    double x;
    double y;
    double z;

};

struct matrix3x3
{

    double x0;
    double y0;
    double z0;
    double x1;
    double y1;
    double z1;
    double x2;
    double y2;
    double z2;

};

unsigned int inperiod(unsigned int frame, unsigned int start, unsigned int end);
