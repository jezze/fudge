# ARM notes

ARM port is only in early development.

## Build

make ARCH=arm TARGET=arm-unknown-eabi LOADER=integratorcp
make ARCH=arm TARGET=arm-none-eabi LOADER=integratorcp

## Run in QEMU

qemu-system-arm -machine integratorcp -kernel fudge -serial stdio
