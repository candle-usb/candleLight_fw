# candleLight_gsusb

This is a firmware for stm32f0-based USB-CAN adapters, notably:
- candleLight: https://github.com/HubertD/candleLight
- cantact: http://linklayer.github.io/cantact/
- canable (cantact clone): http://canable.io/

It implements the interface of the mainline linux gs_usb kernel module and 
works out-of-the-box with linux distros packaging this module, e.g. Ubuntu.

Be aware that there is a bug in the gs_usb module in linux<4.5 that can crash the kernel on device removal.

Here is a fixed version that should also work for older kernels:
  https://github.com/HubertD/socketcan_gs_usb

The Firmware also implements WCID USB descriptors and thus can be used on recent Windows versions without installing a driver.
