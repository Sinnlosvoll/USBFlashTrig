/* central configuration file for USBFlashTrig */


/* common command defines, used in usb ctrl request as bRequest */
#define FT_CMD_TRIGGER           ((unsigned char) 0x01)
#define FT_CMD_FLASH_AND_TRIGGER ((unsigned char) 0x02)
#define FT_CMD_LIGHT_ON          ((unsigned char) 0x03)
#define FT_CMD_LIGHT_OFF         ((unsigned char) 0x04)
#define FT_CMD_LIGHT_STATE		 ((unsigned char) 0x05)
#define FT_CMD_FLASH_TIME_SET    ((unsigned char) 0x06)
#define FT_CMD_FLASH_TIME_GET    ((unsigned char) 0x07)


/* host side /dev/<NAME> creation */
#define DEV_NAME "ft"
#define DEFAULT_DEVICE "/dev/" DEV_NAME "1"


/* usb identifiers, the controller uses */
/* these specificly are quite restricted, you might break licenses of included */
/* libraries, if you change them from the defaults values */
#define DEV_VENDOR_CLASS	0x16c0
#define DEV_PRODUCT_ID		0x05dc



/* the hardware ports on the controller for flash and trigger */
#define PORT_TRIGGER B
#define PIN_TRIGGER  3

#define PORT_FLASH B
#define PIN_FLASH  2



/* if the flash and/or trigger output are active at low level,
 uncomment the following two lines */
// #define TRIGGER_ACTIVE_IS_LOW
// #define FLASH_ACTIVE_IS_LOW