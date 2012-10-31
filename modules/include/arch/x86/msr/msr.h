struct msr_data
{

    unsigned int low;
    unsigned int high;

};

void msr_set(unsigned int msr, struct msr_data *data);
void msr_get(unsigned int msr, struct msr_data *data);
