#define I2C_BUS                         0x0006
#define I2C_TRANSFER_OUT                16
#define I2C_TRANSFER_IN                 512

struct i2c_transfer
{

    unsigned int address;
    unsigned char out[I2C_TRANSFER_OUT];
    unsigned int outcount;
    unsigned char in[I2C_TRANSFER_IN];
    unsigned int incount;
    unsigned int abort;
    unsigned int timeout;

};

unsigned int i2c_getaddress(unsigned int id);
unsigned int i2c_ishid(unsigned int id);
unsigned int i2c_runtransfer(struct i2c_transfer *transfer);
void i2c_wait(unsigned int count);
