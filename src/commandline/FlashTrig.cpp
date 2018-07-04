#include <iostream>

using namespace std;

class FlashTrig
{
private:
	libusb_device_handle *handle;
	libusb_context *context = NULL;
	int usbTimeout = 5000;
	int usbCount = 128;
	void queryDevice(int command, int count);
	bool sendToDevice(int command);
	bool sendToDevice(int command, int usbValue);
	unsigned char rxBuffer[2];

public:
	FlashTrig();
	void setLight(bool on);
	void trigger();
	void flashAndTrigger();
	uint16_t getFlashTime();
	void setFlashTime(uint16_t flashTime);
	bool lightState();
	~FlashTrig();
	bool isOkay;
	
};

FlashTrig::FlashTrig() {

	libusb_device **devs = NULL;
	int ret;


	ret = libusb_init(&(this->context));
	if (ret < 0)
	{
		cerr << "libusb_init failed" << endl;
		this->isOkay = false;
		return;
	}
	
	size_t list;
	list = libusb_get_device_list(this->context, &devs);

	if (list < 0)
	{
		cerr << "Error in getting device list" << endl;
		libusb_free_device_list(devs, 1);
		libusb_exit(this->context);
		this->isOkay = false;
		exit(1);
	}


	this->handle = libusb_open_device_with_vid_pid(this->context, DEV_VENDOR_CLASS, DEV_PRODUCT_ID);

	if (this->handle == NULL)
	{
		cerr << "Could not find flashtrig device" << endl;
		this->isOkay = false;
		return;
	}

	libusb_free_device_list(devs, 1);

	// find out if kernel driver is attached
	if (libusb_kernel_driver_active(this->handle, 0) == 1)
	{
		cout << "Kernel driver is active" << endl;
		if (libusb_detach_kernel_driver(this->handle, 0) == 0)
		{
			cout << "Kernel driver detached" << endl;
		} else {
			cout << "Error detaching kernel driver" << endl;
		}
	}

	ret = libusb_claim_interface(this->handle, 0); // claim interface 0 of device
	if (ret < 0)
	{
		cerr <<  "could not claim flashtrig interface" << endl;
		this->isOkay = false;
		return;
	}
	this->isOkay = true;

}




bool FlashTrig::lightState() {
	this->queryDevice(FT_CMD_LIGHT_STATE, 1);
	if (this->rxBuffer[0] == 0x01) {
		return true;
	}
	return false;
}

void FlashTrig::setFlashTime(uint16_t flashTime) {

	this->sendToDevice(FT_CMD_FLASH_TIME_SET, flashTime);
	return;
}

void FlashTrig::trigger() {

	this->sendToDevice(FT_CMD_TRIGGER);
	return;
}

void FlashTrig::setLight(bool on) {

	if (on)	{
		this->sendToDevice(FT_CMD_LIGHT_ON);
	} else {
		this->sendToDevice(FT_CMD_LIGHT_OFF);
	}
	return;
}

void FlashTrig::flashAndTrigger() {

	this->sendToDevice(FT_CMD_FLASH_AND_TRIGGER);
	return;
}

uint16_t FlashTrig::getFlashTime() {

	this->queryDevice(FT_CMD_FLASH_TIME_GET, 2);

	if (this->isOkay){
		return (uint16_t)((this->rxBuffer[0] << 8) + this->rxBuffer[1]);
	}
	return -1;
}

FlashTrig::~FlashTrig() {

	libusb_release_interface(this->handle, 0);
	libusb_exit(this->context);
}


bool FlashTrig::sendToDevice(int command, int usbValue) {

	int requestType, sentBytes;
	static int usbDirection, usbType, usbRecipient, usbRequest, usbIndex; /* arguments of control transfer */

	usbDirection = 0; 	// [out* in]
	usbType = 2; 		// [standard class vendor* reserved]
	usbRecipient = 0; 	// [device* interface endpoint other]
	usbRequest = command;
	usbIndex = 1; 
	requestType = ((usbDirection & 1) << 7) | ((usbType & 3) << 5) | (usbRecipient & 0x1f); // USB standard § 9.3
	
	sentBytes = libusb_control_transfer(this->handle, requestType, usbRequest, usbValue, usbIndex, NULL, 0, usbTimeout);
	if (sentBytes == 0)
	{
		this->isOkay = true;
		return true;
	}
	this->isOkay = false;
	return false;
}

bool FlashTrig::sendToDevice(int command) {
	return this->sendToDevice(command, 1);
}

void FlashTrig::queryDevice(int command, int count) {

	int requestType, recBytes;
	static int usbDirection, usbType, usbRecipient, usbRequest, usbValue, usbIndex; /* arguments of control transfer */

	usbDirection = 1; 	// [out in*]
	usbType = 2; 		// [standard class vendor* reserved]
	usbRecipient = 0;	// [device* interface endpoint other]
	usbRequest = command;
	usbValue = 1;
	usbIndex = 1;
	requestType = ((usbDirection & 1) << 7) | ((usbType & 3) << 5) | (usbRecipient & 0x1f); // USB standard § 9.3

	recBytes = libusb_control_transfer(this->handle, requestType, usbRequest, usbValue, usbIndex, this->rxBuffer, usbCount, usbTimeout);
	if (recBytes != count) {
		this->isOkay = false;
		return;
	} 
	this->isOkay = true;
	return;
}