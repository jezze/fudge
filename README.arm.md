# ARM notes

ARM port is only in early development.

## Conigure and Build

make arm-integratorcp-none
make

## Run in QEMU

qemu-system-arm -machine integratorcp -kernel fudge -serial stdio
