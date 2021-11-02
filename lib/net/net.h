unsigned char net_load8(unsigned char data[1]);
unsigned short net_load16(unsigned char data[2]);
unsigned int net_load32(unsigned char data[4]);
void net_save8(unsigned char data[1], unsigned char num);
void net_save16(unsigned char data[2], unsigned short num);
void net_save32(unsigned char data[4], unsigned int num);
