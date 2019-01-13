Sample repository to get started with STM32F0
=============================================

Setup enviroment section
========================

In order to use this repository, the user shall install the following
resources and utilities.

## Install toolchain prerequisities

Some of the requirements for installing and using this software are:
- Update the system
- Installation git
- Installation autoreconf & libusb
- Installation cmake
- Installation gtk

## Install toolchain

### Installation compiler and debbuger

#### ARM developer

The GNU Embedded Toolchain for ARM is a ready-to-use, open source suite of
tools for C, C++ and Assembly programming targeting ARM Cortex-M and Cortex-R
family of processors.

The PPA we want in this case is from the GCC ARM Embedded Maintainer’s
team.

##### Linux (Ubuntu)

```
sudo add-apt-repository ppa:team-gcc-arm-embedded/ppa
sudo apt-get update
sudo apt-get install gcc-arm-embedded
```

##### Linux (Fedora)

```
sudo yum install arm-none-eabi-gcc-cs
sudo yum install arm-none-eabi-binutils-cs
sudo yum install arm-none-eabi-newlib
```

##### MacOS

```
brew tap ArmMbed/homebrew-formulae
brew install arm-none-eabi-gcc
```

If it installs correctly, doing autocomplete in the terminal like so:

```
arm-none (press tab twice)
```

You should see something similar like:

```
arm-none-eabi-addr2line   arm-none-eabi-gcc-ar      arm-none-eabi-nm
arm-none-eabi-ar          arm-none-eabi-gcc-nm      arm-none-eabi-objcopy
arm-none-eabi-as          arm-none-eabi-gcc-ranlib  arm-none-eabi-objdump
arm-none-eabi-c++         arm-none-eabi-gcov        arm-none-eabi-ranlib
arm-none-eabi-c++filt     arm-none-eabi-gcov-dump   arm-none-eabi-readelf
arm-none-eabi-cpp         arm-none-eabi-gcov-tool   arm-none-eabi-size
arm-none-eabi-elfedit     arm-none-eabi-gdb         arm-none-eabi-strings
arm-none-eabi-g++         arm-none-eabi-gprof       arm-none-eabi-strip
arm-none-eabi-gcc         arm-none-eabi-ld
arm-none-eabi-gcc-7.3.1   arm-none-eabi-ld.bfd
```

### OpenOCD (optional)

OpenOCD is a free on chip debug solution for targets based on ARM. It is a server
which opens a GDB remote target port and a Telnet port.

In order to install OpenOCD, the code necesary for instalation could be found
in [OpenOCD](https://sourceforge.net/projects/openocd/files/openocd/0.10.0/).

The following commands generate and install the program.

```
 ./configure [options]
 make
 sudo make install
```

### Installation st-link (recommended)

Stlink package is the software for the ST-Link programmer that works with many
ST boards. On the discovery boards, the programmer is embedded at the
top. You just need to make sure you plug in your mini-USB cable into the
center-most port that is labeled ST-Link.

First, clone the repository st-link.

```
git clone https://github.com/texane/stlink.git
```

Every thing can be built from the top of directory.

```
cd stlink
make
cd build/Release
make install
```

To install it run the following command with root permission.

```
sudo make install
```

For some reason, the supplementary library is often not installed correctly,
thus Linux users should move the library manually.

```
sudo cp libusb.so.1 /usr/lib/
```

Stlink installation requires cmake, so if you don't have one:

#### Linux

```
sudo apt-get install cmake
```

#### MacOs

```
brew install cmake
```

In Fedora, st-link software can be installed from repository:

#### Linux (Fedora)

```
sudo yum install stlink
```
