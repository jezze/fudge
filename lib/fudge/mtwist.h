#define MTWIST_VECTORS                  624

struct mtwist_state
{

    unsigned int vectors[MTWIST_VECTORS];
    int current;

};

void mtwist_seed1(struct mtwist_state *state, unsigned int seed);
void mtwist_seed2(struct mtwist_state *state, unsigned int seed);
unsigned int mtwist_rand(struct mtwist_state *state);
