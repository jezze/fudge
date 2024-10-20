#include <fudge.h>
#include <abi.h>

#define R(mul, shift, x, y) \
    f = x; \
    x -= (mul * y) >> shift; \
    y += (mul * f) >> shift; \
    f = (3145728 - x * x - y * y) >> 11; \
    x = x * f >> 10; \
    y = y * f >> 10;

static char b[1760];
static char z[1760];

static void setmemto(char *buffer, int value, unsigned int length)
{

    unsigned int i;
    char *bp = buffer;

    for (i = 0; i < length; i++)
        bp[i] = value;

}

static void onmain(unsigned int source, void *mdata, unsigned int msize)
{

    char sequence[2] = {0x1B, 'c'};
    int sA = 1024;
    int cA = 0;
    int sB = 1024;
    int cB = 0;
    int f;

    lookup2("timer-service", "timer:0/0");
    channel_send(0, option_getdecimal("timer-service"), EVENT_LINK);

    while (channel_process(0))
    {

        int sj = 0;
        int cj = 1024;
        int j;
        int k;

        setmemto(b, 32, 1760);
        setmemto(z, 127, 1760);

        for (j = 0; j < 90; j++)
        {

            int si = 0;
            int ci = 1024;
            int i;

            for (i = 0; i < 324; i++)
            {

                int R1 = 1;
                int R2 = 2048;
                int K2 = 5120 * 1024;
                int x0 = R1 * cj + R2;
                int x1 = ci * x0 >> 10;
                int x2 = cA * sj >> 10;
                int x3 = si * x0 >> 10;
                int x4 = R1 * x2 - (sA * x3 >> 10);
                int x5 = sA * sj >> 10;
                int x6 = K2 + R1 * 1024 * x5 + cA * x3;
                int x7 = cj * si >> 10;
                int x = 40 + 30 * (cB * x1 - sB * x4) / x6;
                int y = 12 + 15 * (cB * x4 + sB * x1) / x6;
                int N = (((-cA * x7 - cB * (((-sA * x7) >> 10) + x2) - ci * ((cj * sB) >> 10)) >> 10) - x5) >> 7;
                int o = x + 80 * y;
                char zz = (x6 - K2) >> 15;

                if (y > 0 && y < 22 && x > 0 && x < 80 && zz < z[o])
                {

                    z[o] = zz;
                    b[o] = ".,-~:;=!*#$@"[N > 0 ? N : 0];

                }

                R(5, 8, ci, si)

            }

            R(9, 7, cj, sj)

        }

        R(5, 7, cA, sA)
        R(5, 8, cB, sB)

        channel_send_buffer(0, source, EVENT_DATA, 2, sequence);

        for (k = 0; k < 1760; k += 80)
        {

            char *offset = b + k;
            unsigned int count = 80;

            channel_send_fmt2(0, source, EVENT_DATA, "%w\n", offset, &count);

        }

    }

    channel_send(0, option_getdecimal("timer-service"), EVENT_UNLINK);

}

void init(void)
{

    option_add("timer-service", "");
    channel_bind(EVENT_MAIN, onmain);

}

