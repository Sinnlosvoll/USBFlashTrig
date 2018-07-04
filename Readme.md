# USBFlashTrig
This project provides the ability to control a remote triggerable camera and an accompanying light for computer controlled image capture.

## Overview
The USBFlashTrig system consists of 3 layers and 4 parts:
-  a linux kernel module or a userspace libusb program
-  a controller 
-  a hardware interface board
The hardware interface board is responsible for the analog actuation of the lights as well as providing the correct electrical specifications for the camera triggering.
The kernel module exposes sysfs-files that correspond to the functions of the controller.

### Kernel Module
The kernel module must be compiled and loaded by executing
```
cd src/module
make
sudo make load
```
As part of the USB Subsystem on Linux the kernel module will automatically assume ownership of any connected flashtrig controller and provide the following files in sysfs:
- trigger
    + (W) triggers the camera to take a picture
- flash
    + (W) starts the flash and triggers the camera
- light_on
    + (W) turns on the flash light
- light_off
    + (W) turns off the flash light
- light_state
    + (R\) represents the current light state, on or off
- flash_time
    + (R/W) the time of the flash light to be on when flashing

`Trigger`, `flash`, `light_on` and `light_off` accept any input:
```
echo 1 > trigger
echo asdjhaksjdhaksjdh > flash
```
`light_state` and `flash_time` can be read and return the set values:
```
cat flash_time % return e.g. 500 meaning 500ms
cat light_state % returns [0|1], depending on the light state [off|on]
```
The `flash_time` can be set with:
```
echo 20000 > flash_time
```
this turns the light on for 20 seconds, when flash is executed.

### Userspace libusb program
This userspace utility allows control of the FlashTrig controller without sysfs, and serves as an example on how to integrate it into other programs.
It consists of a C++ class `FlashTrig.cpp` for the FlashTrig controller and a corresponding argument parser `control.cpp`. **The program needs r/w rights on the controller (e.g. sudo)**
To compile:
```
cd src/commandline && make
```
For usage:
```
./flashtrig --help
```

### Controller
The controller is a modified usbasp. To flash the firmware:
```
cd src/device
make
sudo make flash
```
It uses interrupt timers to achieve 1ms resolution on the flash time, up to 65.535 seconds. The control with the host pc is accomplished using the V-USB library from OBdev and one usb control endpoint.


### Hardware interface board
This board is effectively the driver stage of the controller. It has a resistor arrangement to remote trigger a Panasonic GH-2 and a power MOSFET to control the power line of a DC-powered light.
