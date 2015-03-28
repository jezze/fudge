#include <fudge.h>

#define _Complex_I                      0

#define WIDTH                           320
#define HEIGHT                          200
#define ZOOM                            2.0

struct complex
{

    double r;
    double i;

};

double creal(struct complex c)
{

    return c.r;

}

double cimag(struct complex c)
{

    return c.i;

}

double cabs(struct complex c)
{

    return 0;

}

struct complex caddc(struct complex c1, struct complex c2)
{

    struct complex c;

    c.r = c1.r + c2.r;
    c.i = c1.i + c2.i;

    return c;

}

struct complex csubc(struct complex c1, struct complex c2)
{

    struct complex c;

    c.r = c1.r - c2.r;
    c.i = c1.i - c2.i;

    return c;

}

struct complex cmuli(struct complex c1, int n)
{

    struct complex c;

    c.r = c1.r * n;
    c.i = c1.i * n;

    return c;

}

struct complex cmulc(struct complex c1, struct complex c2)
{

    struct complex c;

    c.r = (c1.r * c2.r) - (c1.i * c2.i);
    c.i = (c1.i * c2.r) + (c1.r * c2.i);

    return c;

}

struct complex cpow(struct complex c1, unsigned int t)
{

    return cmulc(c1, c1);

}

struct complex iterate(struct complex c, unsigned int iterations)
{

    struct complex z;
    unsigned int i;

    z.r = c.r;
    z.i = c.i;

    for (i = 0; i < iterations; i++)
        z = caddc(cpow(z, 2), c);

    return z;

}

static void render(double rcenter, double icenter, unsigned int iterations, double zoom, double granularity)
{

    unsigned char buffer[320];
    double rsize = WIDTH / 2;
    double isize = HEIGHT / 2;
    int x, y;

    for (y = 0; y < HEIGHT; y++)
    {   

        memory_clear(buffer, 320);

        for (x = 0; x < WIDTH; x++)
        {

            double rpoint = x;
            double ipoint = y;
            struct complex c;
            struct complex z;
            
            c.r = (rcenter + ((rpoint - rsize) / zoom));
            c.i = (icenter + ((ipoint - isize) / zoom));

            z = iterate(c, iterations);

            if (cabs(z) >= granularity)
                buffer[x] = 0xFF;

        }

        call_write(CALL_PO, y * WIDTH, WIDTH, 1, buffer);

    }

}

static void handle(char c)
{

    switch (c)
    {

        case 'q':

            call_exit();

            break;

        default:

            break;

    }

}

static void poll()
{

    unsigned char buffer[FUDGE_BSIZE];
    unsigned int count;
    unsigned int i;

    for (;;)
    {

        count = call_read(CALL_P0, 0, 1, FUDGE_BSIZE, buffer);

        for (i = 0; i < count; i++)
            handle(buffer[i]);

    }

}

void main()
{

    unsigned int iterations = 32;
    double x = -0.5;
    double y = 0;
    double granularity = 8.0;
    double zoom = WIDTH / 4.0;

    render(x, y, iterations, zoom, granularity);
    poll();

}

