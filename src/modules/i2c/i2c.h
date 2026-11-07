#define I2C_BUS                         0x0006
#define I2C_TRANSFER_OUT                16
#define I2C_TRANSFER_IN                 1024

struct i2c_transfer
{

    unsigned int id;
    unsigned char out[I2C_TRANSFER_OUT];
    unsigned int outcount;
    unsigned char in[I2C_TRANSFER_IN];
    unsigned int incount;
    unsigned int abort;
    unsigned int timeout;

};

struct i2c_pad
{

    unsigned int found;
    unsigned int group;
    unsigned int pad;

};

unsigned int i2c_getaddress(unsigned int id);
unsigned int i2c_gethidregister(unsigned int id);
struct i2c_pad i2c_getpad(unsigned int id);
unsigned int i2c_ishid(unsigned int id);
unsigned int i2c_runtransfer(struct i2c_transfer *transfer);
void i2c_wait(unsigned int id, unsigned int count);
