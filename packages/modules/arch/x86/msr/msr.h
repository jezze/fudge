struct msr_data
{

    unsigned int data0;
    unsigned int data1;

};

void msr_set(unsigned int msr, struct msr_data *data);
void msr_get(unsigned int msr, struct msr_data *data);
