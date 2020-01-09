# candleLight_gsusb
[![Build Status](https://travis-ci.org/candle-usb/candleLight_fw.svg?branch=master)](https://travis-ci.org/candle-usb/candleLight_fw)

This is a firmware for stm32f0-based USB-CAN adapters, notably:
- candleLight: https://github.com/HubertD/candleLight
- cantact: http://linklayer.github.io/cantact/
- canable (cantact clone): http://canable.io/
- USB2CAN: https://github.com/roboterclubaachen/usb2can
- CANAlyze: https://kkuchera.github.io/canalyze/

It implements the interface of the mainline linux gs_usb kernel module and
works out-of-the-box with linux distros packaging this module, e.g. Ubuntu.

## Known issues

Be aware that there is a bug in the gs_usb module in linux<4.5 that can crash the kernel on device removal.

Here is a fixed version that should also work for older kernels:
  https://github.com/HubertD/socketcan_gs_usb

The Firmware also implements WCID USB descriptors and thus can be used on recent Windows versions without installing a driver.

## Building

Building requires arm-none-eabi-gcc toolchain.

```shell
sudo apt-get install gcc-arm-none-eabi

mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi-8-2019-q3-update.cmake

# or,
# cmake-gui ..
# don't forget to specify the cmake toolchain file before configuring.

make canalyze_fw # one of candleLight_fw / usb2can_fw / cantact_fw / canalyze_fw / canable_fw
# alternately, each board target may be disabled as cmake options

```

## Flashing

Flashing candleLight on linux: (source: [https://wiki.linklayer.com/index.php/CandleLightFirmware](https://wiki.linklayer.com/index.php/CandleLightFirmware))
- Flashing requires the dfu-util tool. On Ubuntu, this can be installed with `sudo apt install dfu-util`.
- compile as above, or download the current binary release: gsusb_cantact_8b2b2b4.bin
- Disconnect the USB connector from the CANtact, short the BOOT pins, then reconnect the USB connector. The device should enumerate as "STM32 BOOTLOADER".
- Flash the device with: `sudo dfu-util --dfuse-address -d 0483:df11 -c 1 -i 0 -a 0 -s 0x08000000 -D CORRECT_FIRWARE.bin` where CORRECT_FIRWARE is the name of the desired .bin.
- Disconnect the USB connector, un-short the BOOT pins, and reconnect. The device is now flashed!
- If dfu-util fails due to permission issues on Linux, you may need additional udev rules. Consult your distro's documentation and see `70-candle-usb.rules` provided here.
