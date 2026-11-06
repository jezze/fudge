struct gpio_pad
{

    unsigned long community;
    unsigned long dw0;
    unsigned int group;
    unsigned int bit;

};

struct gpio_pad gpio_findpad(unsigned int port, unsigned int pad, unsigned int group, unsigned int bit);
unsigned int gpio_islow(struct gpio_pad *pad);
unsigned int gpio_setinterrupt(struct gpio_pad *pad, unsigned int (*routine)(void));
void gpio_unsetinterrupt(struct gpio_pad *pad);
