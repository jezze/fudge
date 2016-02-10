#include <fudge.h>
#include <kernel.h>
#include <modules/system/system.h>

#define MTWIST_VECTORS                  624
#define MTWIST_SEED                     4357
#define MTWIST_RECURRENCE               397
#define MTWIST_COMBINE_BITS(x, y)       (((x) & 0x80000000) | ((y) & 0x7FFFFFFF))
#define MTWIST_MATRIX_MULTIPLY(x, y)    ((x) ^ ((y) >> 1) ^ decider[(y) & 1])

struct mtwist_state
{

    unsigned int vectors[MTWIST_VECTORS];
    int current;

};

static unsigned int decider[2] = {0x00000000, 0x9908B0DF};
static struct mtwist_state normal;
static struct system_node root;

static void refresh(struct mtwist_state *state)
{

    unsigned int *current;
    unsigned int value1;
    unsigned int value2;
    int i;

    current = &state->vectors[MTWIST_VECTORS - 1];
    value1 = *current;

    for (i = (MTWIST_VECTORS - MTWIST_RECURRENCE) / 2; --i >= 0;)
    {

        current -= 2;
        value2 = current[1];
        value1 = MTWIST_COMBINE_BITS(value1, value2);
        current[2] = MTWIST_MATRIX_MULTIPLY(current[-MTWIST_RECURRENCE + 2], value1);
        value1 = current[0];
        value2 = MTWIST_COMBINE_BITS(value2, value1);
        current[1] = MTWIST_MATRIX_MULTIPLY(current[-MTWIST_RECURRENCE + 1], value2);

    }

    value2 = *--current;
    value1 = MTWIST_COMBINE_BITS(value1, value2);
    current[1] = MTWIST_MATRIX_MULTIPLY(current[-MTWIST_RECURRENCE + 1], value1);

    for (i = (MTWIST_RECURRENCE - 1) / 2; --i >= 0;)
    {

        current -= 2;
        value1 = current[1];
        value2 = MTWIST_COMBINE_BITS(value2, value1);
        current[2] = MTWIST_MATRIX_MULTIPLY(current[MTWIST_VECTORS - MTWIST_RECURRENCE + 2], value2);
        value2 = current[0];
        value1 = MTWIST_COMBINE_BITS(value1, value2);
        current[1] = MTWIST_MATRIX_MULTIPLY(current[MTWIST_VECTORS - MTWIST_RECURRENCE + 1], value1);

    }

    value1 = MTWIST_COMBINE_BITS(value2, state->vectors[MTWIST_VECTORS - 1]);
    *current = MTWIST_MATRIX_MULTIPLY(current[MTWIST_VECTORS - MTWIST_RECURRENCE], value1);
    state->current = MTWIST_VECTORS;

}

static void seed1(struct mtwist_state *state, unsigned int seed)
{

    int i;

    state->vectors[MTWIST_VECTORS - 1] = seed & 0xFFFFFFFF;

    for (i = MTWIST_VECTORS - 2; i >= 0; i--)
        state->vectors[i] = (69069 * state->vectors[i + 1]) & 0xFFFFFFFF;

    state->current = MTWIST_VECTORS;

    refresh(state);

}

/*
static void seed2(struct mtwist_state *state, unsigned int seed)
{

    int i;

    state->vectors[MTWIST_VECTORS - 1] = seed & 0xFFFFFFFF;

    for (i = MTWIST_VECTORS - 2; i >= 0; i--)
    {

        unsigned int nextval = state->vectors[i + 1] >> 30;

        nextval ^= state->vectors[i + 1];
        nextval *= 1812433253;
        nextval += (MTWIST_VECTORS - 1) - i;
        state->vectors[i] = nextval & 0xFFFFFFFF;

    }

    state->current = MTWIST_VECTORS;

    refresh(state);

}
*/

static unsigned int rand(struct mtwist_state *state)
{

    unsigned int value = state->vectors[--state->current];

    value ^= (value >> 11);
    value ^= (value << 7) & 0x9D2C5680;
    value ^= (value << 15) & 0xEFC60000;
    value ^= (value >> 18);

    return value;

}

/*
static double mtwist_drand(struct mtwist_state *state)
{

    double conv = 1.0;
    unsigned int i;

    for (i = 0; i < 32; i++)
        conv /= 2.0;

    return rand(state) * conv;

}
*/

static unsigned int root_read(struct system_node *self, struct list_item *link, unsigned int offset, unsigned int count, void *buffer)
{

    unsigned int x = rand(&normal);

    return memory_read(buffer, count, &x, 4, offset);

}

void module_init(void)
{

    seed1(&normal, MTWIST_SEED);
    system_initnode(&root, SYSTEM_NODETYPE_NORMAL, "mtwist");

    root.read = root_read;

}

void module_register(void)
{

    system_registernode(&root);

}

void module_unregister(void)
{

    system_unregisternode(&root);

}

