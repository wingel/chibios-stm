STM32 firmwares based on ChibiOS
================================

I've built a couple of small PCBs with different ST Micro STM32 MCUs
on them, such as my [STM32F072 breakout
board](https://github.com/wingel/stm32f072-breakout).  These boards
need some kind of firmware to do something useful.

[ChibiOS-RT](http://www.chibios.org/) is a small RTOS which has
support for most STM32 MCUs, including a USB stack which can make the
MCU appear as a virtual COM port (CDC ACM) on both Windows and Linux.
This means that ChibiOS makes it very easy to start actually doing
something with the hardware I've built.

[This project](https://github.com/wingel/chibios-stm) is a collection
of small test firmwares for the PCBs I've designed and built.  It's
nothing fancy, most firmwares just blink a LED and start up a virtual
COM port where one can use the built in "shell" to run small commands
on the device.

It's very easy to extend the ChibiOS shell with new commands, making
it possible to quickly verify that a piece of hardware is working the
way it should.

Getting Started
===============

Prerequisites
-------------

Get a [STM32F072 breakout board](stm32f072-breakout).  You'll probably
have to have someone else manufacture the naked PCB for you but you
should be able to solder the components yourself.  The STM32F072 MCU
and the USB connector can be a bit tricky to solder, but other than
that there are only a dozen small components needed.

Install a recent version of Ubuntu Linux on a PC.  Personally I use
[Linux Mint 17.2](http://www.linuxmint.com/download.php) which based
on [Ubuntu 14.04](http://www.ubuntu.com/download/desktop).  If you run
Windows or OSX the easiest way to get started is probably to install
Ubuntu in a virtual machine, but I'm not sure if it will be program
the board over USB using a virtual machine.

When you have installed Ubuntu, start a terminal and make sure the
system is up to date:

    sudo apt-get update
    sudo apt-get upgrade

Install some tools and libraries needed by this project:

    sudo apt-get install git automake autoconf make pkg-config libusb-1.0-0-dev

Install a toolchain, that is, a C compiler, linker, C library and some
other tools needed to build applications for Linux:

    sudo apt-get install gcc binutils libc6-dev

Install a toolchain for ARM needed to build applications for the ARM
core in the STM32 MCU:

    sudo apt-get install gcc-arm-none-eabi libnewlib-arm-none-eabi

Getting the source code
-----------------------

Use git to download a copy (clone) of the source code from github:

    git clone https://github.com/wingel/chibios-stm.git

Use git to download the source code for some other projects
(submodules) this project depends on:

    cd chibios-stm
    git submodule init
    git submodule update

Building
--------

Build dfu-util which is needed to program a STM32 device over USB:

    cd dfu-util
    ./autogen.sh
    ./configure
    make
    cd ..

Build the firmware for the STM32F072 breakout board:

    cd stm32f072-breakout
    ./build.sh

Connect the STM32F072 breakout board to your PC.  Put it in Device
Firmware Update (DFU) mode by keeping the BOOT button pressed while
pressing and releasing the RESET button.

If you look at the kernel logs.

    dmesg | tail

You should see some messages about new a USB device like this:

    usb 4-1.2: New USB device found, idVendor=0483, idProduct=df11
    usb 4-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
    usb 4-1.2: Product: STM32  BOOTLOADER
    usb 4-1.2: Manufacturer: STMicroelectronics
    usb 4-1.2: SerialNumber: FFFFFFFEFFFF

Start the programming with:

    ./prog.sh

When the programming has finished the board should reboot and the blue
LED should start blinking.  If you want make sure that it really is
your new firmware that's running, modify "main.c" and change the
chThdSleepMilliseconds delays in the "blink" thread function at the
beginning of the file, build and program the board again and verify
that the blink rate changes.

If you look at the kernel logs now the board should also show as a USB
virtual COM port named ttyACM-something:

    usb 4-1.2: New USB device found, idVendor=0483, idProduct=5740
    usb 4-1.2: New USB device strings: Mfr=1, Product=2, SerialNumber=3
    usb 4-1.2: Product: STM32
    usb 4-1.2: Manufacturer: STMicroelectronics
    usb 4-1.2: SerialNumber: 000000000000000000000000
    cdc_acm 4-1.2:1.0: ttyACM3: USB ACM device

Start a terminal program and connect to the virtual COM port.  I
usually use [my own little terminal program for Linux,
tt](https://github.com/wingel/tt), but you could also use minicom or
Putty which is available for both Linux and Windows.  You can now
start issuing commands to the ChibiOS shell, for example, try the
"help" or "info" commands:

    ChibiOS/RT Shell
    ch> help
    Commands: help exit info systime reset
    ch> info
    Kernel:       3.0.2
    Compiler:     GCC 4.8.2
    Architecture: ARMv6-M
    Core Variant: Cortex-M0
    Port Info:    Preemption through NMI
    Platform:     STM32F072xB Entry Level Medium Density devices
    Board:        ST STM32F072B-Discovery
    Build time:   Aug 24 2015 - 21:14:14
    ch>

You can now start extending the firmware with new commands.  To see
how to do this, open at main.c and look at the function "cmd_reset"
and the "commands" table.  For example, to see an example on how to
extend the [stm32f072-breakout](stm32f072-breakout) code to interface
with a DHT11 or DHT22 temperature sensor, look at the
[stm32f072-breakout-dht](stm32f072-breakout-dht) directory.
