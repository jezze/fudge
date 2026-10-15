#define USB_BUS                         0x0005
#define USB_REQUEST_OUT                 0x00
#define USB_REQUEST_IN                  0x80
#define USB_REQUEST_STANDARD            0x00
#define USB_REQUEST_CLASS               0x20
#define USB_REQUEST_DEVICE              0x00
#define USB_REQUEST_INTERFACE           0x01

unsigned int usb_getclass(unsigned int id);
unsigned int usb_getsubclass(unsigned int id);
unsigned int usb_getprotocol(unsigned int id);
unsigned int usb_getinterface(unsigned int id);
unsigned int usb_control(unsigned int id, unsigned char type, unsigned char request, unsigned short value, unsigned short index, unsigned short length, void *data);
unsigned int usb_listen(unsigned int id, void (*callback)(unsigned int id, void *data, unsigned int count));
void usb_unlisten(unsigned int id);
