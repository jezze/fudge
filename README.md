# Fudge Operating System

http://www.fudgeos.com/

## Overview

Fudge is a concurrent-focused operating system. Fudge is different from most
other operating systems in that it adheres strongly to the actor model. The
kernel and every task in the system communicates by posting messages to each
other's mailboxes.

The way this is implemented makes concurrency feel like a natural part of your
programs especially in comparison to the asynchronous programming models used in
file-centric operating systems.

Fudge tries to keep as little state as possible inside the kernel itself to
avoid complexity. There is for instance no out-of-the-box support for dynamic
memory allocation. Drivers are implemented as loadable kernel modules but they
are typically only big enough to support only the most basic of functionality,
leaving the higher level logic to be done in userspace because there is no best
solution to every use case so a user can choose to implement things differently
if they wish.

Each program has a channel which it uses to read messages from it's mailbox. For
each type of message a program can register a callback that will be run when a
message of a certain type appears. Messages without a registered callback will
be dropped. It is also possible to poll messages manually from the channel.

## Architectures

Fudge runs on several architectures. Each one has a README with its cross
compiler, configurations and how to run it:

| Architecture | Configurations         | Cross compiler      | Notes              |
|--------------|------------------------|---------------------|--------------------|
| x86          | x86-mboot, x86-mboot2  | i386-unknown-elf    | README.x86.md      |
| x86_64       | x86_64-mboot2          | x86_64-unknown-elf  | README.x86_64.md   |
| arm          | arm-integratorcp       | arm-unknown-eabi    | README.arm.md      |
| riscv64      | riscv64-virt           | riscv64-unknown-elf | README.riscv64.md  |
| m68k         | m68k-m68kvirt          | m68k-unknown-elf    | README.m68k.md     |

## Getting the source

```sh
$ git clone https://github.com/jezze/fudge.git
$ cd fudge
```

## Building a cross compiler (crosstool-ng)

Fudge is always built with a cross compiler for its target, also for x86 and
x86_64 on an x86 host. Building one can be a tedious task but luckily there is
a tool called crosstool-ng that automates the entire process.

Download and install crosstool-ng. The recommended version is 1.29.0.

```sh
$ wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.29.0.tar.bz2
$ tar xjf crosstool-ng-1.29.0.tar.bz2
$ cd crosstool-ng-1.29.0
$ ./configure
$ make
$ sudo make install
```

Create a folder for the toolchain in your home directory and cd to it:

```sh
$ mkdir -p ~/temp/<cross compiler>
$ cd ~/temp/<cross compiler>
```

Configure the toolchain by running:

```sh
$ ct-ng menuconfig
```

This starts the configuration tool. Choose the settings your architecture's
README lists. The resulting name (the tuple) must match the cross compiler in
the table above, since that is what the build calls. Exit the configuration
tool by pressing exit a few times and save the settings when asked.

Start building the cross compiler:

```sh
$ ct-ng build
```

Depending on your system this may take some time, usually around 10-15 minutes.
By default it is installed in ~/x-tools/<cross compiler>/. Please refer to the
crosstool-ng website for more information on how to customize your setup.

Before building Fudge, add the cross compiler to your $PATH. Unless you changed
the install path, this should work:

```sh
$ export PATH=~/x-tools/<cross compiler>/bin:$PATH
```

The change to $PATH disappears when you leave the current session, so you will
need to set it again from time to time.

## Building Fudge

First set the configuration, which is written to mk/config.mk, then build.
make help lists the configurations.

```sh
$ make <configuration>
$ make
```

You should now have a kernel called fudge and a ramdisk image called fudge.cpio
in the top directory of your source tree. make clean removes the build of the
current configuration, so run it before switching to another one.

## Running Fudge in QEMU with vmctrl

There is a script called vmctrl that you can use to run Fudge as a virtual
machine in QEMU. -x picks the architecture (x86 if left out).

```sh
$ ./vmctrl run -x <architecture>
```

This will start an instance of Fudge called node1. Some configurations can not
be loaded by QEMU directly and need a disk or an ISO; their README says so.
vmctrl help lists all options. Which storage, network and sound devices an
architecture has is in its README.

The node will not have any networking abilities by default. The simplest way to
enable networking is the -N flag, which adds a network card on QEMU's user mode
networking and does not need root. The node gets the address 10.0.5.1 and QEMU
answers as the router on 10.0.5.80, which are the defaults the network programs
in Fudge use, so it can reach the outside world right away. The host can not
connect to the node this way and nodes can not talk to each other.

```sh
$ ./vmctrl run -x <architecture> -N
```

To let nodes talk to each other, use -t and a name. Your user need to be
allowed to create TAP interfaces on your host machine. Typically using sudo will
allow you to do this.

```sh
$ sudo ./vmctrl run -x <architecture> -t <name>
```

Once you started one or more instances of Fudge, each with a different name you
can bridge their networks together with this command.

```sh
$ sudo ./vmctrl connect
```

Now each virtual machine should be able to find and talk to each other. In
order for them to be able to talk to the outside world you need to set up
network address translation on your host machine. This can be done by issuing
the following command:

```sh
$ sudo ./vmctrl nat eth0
```

Replace eth0 with the name of the network interface that is connected to the
internet. You only need to run this command once unless you reboot your host
machine.

## Disk images

You can create a fudge.qcow2 image and install Fudge on it. First build Fudge,
then create the image:

```sh
$ ./vmctrl disk_create
```

Then set it up. This copies the build onto the image (disk_setup_gpt makes a GPT
instead of an MBR) and usually requires root privileges:

```sh
$ sudo ./vmctrl disk_setup fudge.qcow2
```

-S attaches fudge.qcow2 and -d <disk> another image. The disk is mounted
according to data/config/mount.mq.

## Community

There is a discord server:

Link: https://discord.gg/4nNq6Grhtz

There is also an irc channel:

Server: irc.libera.chat

Channel: #fudge

