#define ISR_ROUTINE_SLOTS               256

enum isr_index
{

    ISR_INDEX_DE                        = 0x00,
    ISR_INDEX_DB                        = 0x01,
    ISR_INDEX_NI                        = 0x02,
    ISR_INDEX_BP                        = 0x03,
    ISR_INDEX_OF                        = 0x04,
    ISR_INDEX_BR                        = 0x05,
    ISR_INDEX_UD                        = 0x06,
    ISR_INDEX_NM                        = 0x07,
    ISR_INDEX_DF                        = 0x08,
    ISR_INDEX_CO                        = 0x09,
    ISR_INDEX_TS                        = 0x0A,
    ISR_INDEX_NP                        = 0x0B,
    ISR_INDEX_SS                        = 0x0C,
    ISR_INDEX_GP                        = 0x0D,
    ISR_INDEX_PF                        = 0x0E,
    ISR_INDEX_MF                        = 0x10,
    ISR_INDEX_AC                        = 0x11,
    ISR_INDEX_MC                        = 0x12,
    ISR_INDEX_XM                        = 0x13

};

void isr_routine();
void isr_disable_pic();
void isr_usermode(unsigned int cs, unsigned int ds, unsigned int ip, unsigned int sp);
void isr_setup(unsigned short selector);
