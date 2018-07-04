// SPDX-License-Identifier: GPL-2.0+
/*
 * USB Flash Trigger Driver 
 *		with inspiration taken from PlayStation 2 Trance Vibrator driver
 *
 * Copyright (C) 2018 Christopher Hofmann <christopherushofmann@googlemail.com>
 */

/* Standard include files */
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/usb.h>
#include "../common/defines.h"

#define DRIVER_AUTHOR "Christopher Hofmann, <christopherushofmann@googlemail.com>"
#define DRIVER_DESC "USB Flash and Trigger Manager"


static const struct usb_device_id id_table[] = {
	{ USB_DEVICE(DEV_VENDOR_CLASS, DEV_PRODUCT_ID) },
	{ },
};

MODULE_DEVICE_TABLE (usb, id_table);

/* tiny struct, as everything is asked from the device and not stored host side */
struct flashtrig {
	struct usb_device *udev;
};


static ssize_t send_cmd(struct device *dev, struct device_attribute *attr, char cmd, size_t count, int16_t *value)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct flashtrig *ft = usb_get_intfdata(intf);
	int retval;

	if (cmd == FT_CMD_FLASH_TIME_SET)
	{
		retval = usb_control_msg(ft->udev, 				// *dev
			usb_sndctrlpipe(ft->udev, 0),				// pipe
			cmd,										// request
			USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_OTHER, // requestType
			*value, /* flash time */					// value
			0, 											// index
			NULL, 										// data
			0, 											// size
			USB_CTRL_GET_TIMEOUT);						// timeout

	} else {

		retval = usb_control_msg(ft->udev, 				// *dev
			usb_sndctrlpipe(ft->udev, 0),				// pipe
			cmd, 										// request
			USB_DIR_OUT | USB_TYPE_VENDOR | USB_RECIP_OTHER, // requestType
			0, 											// value
			0, 											// index
			NULL, 										// data
			0, 											// size
			USB_CTRL_GET_TIMEOUT);						// timeout

	}
	
	return retval;
}

static ssize_t rec_cmd(struct device *dev, struct device_attribute *attr, char cmd, size_t count, int16_t *value)
{
	struct usb_interface *intf = to_usb_interface(dev);
	struct flashtrig *ft = usb_get_intfdata(intf);
	int retval;
	u8 *buf = kmalloc(2, GFP_KERNEL);

	retval = usb_control_msg(ft->udev, 
				usb_rcvctrlpipe(ft->udev, 0), 
				cmd, 
				USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				0, 
				0,
				buf, 
				2,
				USB_CTRL_GET_TIMEOUT);


	if (cmd == FT_CMD_FLASH_TIME_GET)
	{
		*value = buf[1];
		*value |= buf[0] << 8;

	} else if (cmd == FT_CMD_LIGHT_STATE)
	{
		*value = buf[0];
	}

	return retval;
}


static ssize_t flash_time_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int16_t val = -1;
	rec_cmd(dev, attr, FT_CMD_FLASH_TIME_GET, 1, &val);
	if (val == -1)
	{
		return sprintf(buf, "error fetching flash time\n");
	}
	return sprintf(buf, "%d\n", val);
}

static ssize_t light_state_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	int16_t val = 2;
	rec_cmd(dev, attr, FT_CMD_LIGHT_STATE, 1, &val); 
	if (val == 2)
	{
		return sprintf(buf, "error fetching light state\n");
	}
	return sprintf(buf, "%d\n", val);
}



static ssize_t trigger_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	send_cmd(dev, attr, FT_CMD_TRIGGER, 1, 0);
	// ignore user input, this is a binary toggle. just return success
	return count;
}

static ssize_t flash_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	send_cmd(dev, attr, FT_CMD_FLASH_AND_TRIGGER, 1, 0);
	// ignore user input, this is a binary toggle. just return success
	return count;
}

static ssize_t light_on_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	send_cmd(dev, attr, FT_CMD_LIGHT_ON, 1, 0);
	// ignore user input, this is a binary toggle. just return success
	return count;
}

static ssize_t light_off_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	send_cmd(dev, attr, FT_CMD_LIGHT_OFF, 1, 0);
	// ignore user input, this is a binary toggle. just return success
	return count;
}

static ssize_t flash_time_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
	// sets the time of the "flash" being on 16 bit, 1ms resolution
	int16_t value;
	int i, mult;
	value = 0;
	mult = 1;

	for (i = count-2; i >=0 ; i--)
	{	
		value += mult*(buf[i]-48);
		mult *= 10;
	}
	send_cmd(dev, attr, FT_CMD_FLASH_TIME_SET, 1, &value);
	return count;
}


static DEVICE_ATTR_WO(trigger);
static DEVICE_ATTR_WO(flash);
static DEVICE_ATTR_WO(light_on);
static DEVICE_ATTR_WO(light_off);
static DEVICE_ATTR_RO(light_state);
static DEVICE_ATTR_RW(flash_time);


static int ft_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	struct usb_device *udev = interface_to_usbdev(interface);
	struct usb_device_descriptor *buf;

	struct flashtrig *dev;
	int retval;

	dev = kzalloc(sizeof(struct flashtrig), GFP_KERNEL);
	if (!dev) {
		retval = -ENOMEM;
		goto error;
	}

	buf = kmalloc(USB_DT_DEVICE_SIZE, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	kfree(buf);


	dev->udev = usb_get_dev(udev);
	usb_set_intfdata(interface, dev);
	// retval = device_create_file(&interface->dev, &dev_attr_speed);
	retval = device_create_file(&interface->dev, &dev_attr_trigger);
	retval = device_create_file(&interface->dev, &dev_attr_flash);
	retval = device_create_file(&interface->dev, &dev_attr_flash_time);
	retval = device_create_file(&interface->dev, &dev_attr_light_on);
	retval = device_create_file(&interface->dev, &dev_attr_light_off);
	retval = device_create_file(&interface->dev, &dev_attr_light_state);
	if (retval)
		goto error_create_file;

	return 0;

error_create_file:
	usb_put_dev(udev);
	usb_set_intfdata(interface, NULL);
error:
	kfree(dev);
	return retval;
}

static void ft_disconnect(struct usb_interface *interface)
{
	struct flashtrig *dev;

	dev = usb_get_intfdata (interface);
	device_remove_file(&interface->dev, &dev_attr_trigger);
	device_remove_file(&interface->dev, &dev_attr_flash);
	device_remove_file(&interface->dev, &dev_attr_flash_time);
	device_remove_file(&interface->dev, &dev_attr_light_on);
	device_remove_file(&interface->dev, &dev_attr_light_off);
	device_remove_file(&interface->dev, &dev_attr_light_state);
	usb_set_intfdata(interface, NULL);
	usb_put_dev(dev->udev);
	kfree(dev);
}

/* USB subsystem object */
static struct usb_driver ft_driver = {
	.name =			DEV_NAME,
	.probe =		ft_probe,
	.disconnect =	ft_disconnect,
	.id_table =		id_table,
};

module_usb_driver(ft_driver);

MODULE_AUTHOR(DRIVER_AUTHOR);
MODULE_DESCRIPTION(DRIVER_DESC);
MODULE_LICENSE("GPL");