# RISC-V notes

RISC-V port is only in early development.

## Build

make ARCH=riscv TARGET=riscv64-linux-gnu LOADER=medany

## Run in QEMU

qemu-system-riscv64 -machine virt -bios fudge -serial stdio
