struct msr_data
{

    unsigned int eax;
    unsigned int edx;

};

void msr_set(unsigned int msr, struct msr_data *data);
void msr_get(unsigned int msr, struct msr_data *data);
