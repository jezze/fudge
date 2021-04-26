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
are typically only big enough to support only the most basic of functionallity,
leaving the higher level logic to be done in userspace because there is no best
solution to every use case so a user can chose to implement things differently
if they wish.

Each program has a channel which it uses to read messages from it's mailbox. For
each type of message a program can register a callback that will be run when a
message of a certain type appears. Messages without a registered callback will
be dropped. By default the system has callbacks for MAIN, TERM and REDIRECT
messages but these can be overriden. It is possible to poll messages manually
from the channel, either from a certain sender or of a certain type. Anything
that falls outside of that will be automatically dispatched to the default
registered callbacks. It is also possible to dispatch a message manually.

## Building a cross compiler

In order to compile Fudge for x86 we need to have a cross-compiler. Building a
cross-compiler can be a tedious task but luckily there is a tool called
crosstool-ng that automates the entire process for you.

### Installing crosstools-ng

Download and install crosstool-ng. The recommended version is 1.24.0.

```sh
$ wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.24.0.tar.bz2
$ tar xjf crosstool-ng-1.24.0.tar.bz2
$ cd crosstool-ng-1.24.0
$ ./configure
$ make
$ sudo make install
```

The last command must be run with root privileges (by using sudo in this case).

### Configuring crosstools-ng

Create a folder in your home directory called temp and cd to it.

```sh
$ mkdir ~/temp
$ cd ~/temp
```

Configure the cross-compiler environment by running:

```sh
$ ct-ng menuconfig
```

This will start the configuration program. Set the target architecture to x86.

```sh
Target options -> Target Architecture -> x86
```

By default the cross-compiler will be installed in ~/x-tools/i386-unknown-elf/.
You can change this if you want. Please refer to the crosstool-ng website for
more information on how to customize your setup.

Exit the configuration tool by pressing exit a few times. Before you leave the
last screen you will be asked if you want to save the settings. Press yes.

### Building the cross compiler

Start building the cross-compiler:

```sh
$ ct-ng build
```

Depending on your system this may take some time, usually around 10-15 minutes.

## Building Fudge

This requires you to have the cross-compiler built.

### Get the Fudge source code

First you need to download the source code if you haven't done that already.

```sh
$ git clone git://github.com/jezze/fudge.git
$ cd fudge
```

### Set the PATH environment variable

In this example the cross-compiler was installed in
/home/foo/x-tools/i386-unknown-elf/. Change this to match your setup. If you
haven't made any major modifications you just need to change foo to match your
username instead.

```sh
$ export PATH=/home/foo/x-tools/i386-unknown-elf/bin:$PATH
```

The PATH modifications will be gone as soon as you leave the current session or
start another one. This means that you will need to reset it from time to time.

### Building Fudge

Finally we can build Fudge.

```sh
$ make
```

You should now have a binary called fudge and an image called fudge.cpio in the
top directory of your source tree.

## Running Fudge in QEMU

There is a script you can use to start Fudge in QEMU called vmctrl. The easiest
way to run fudge in QEMU is to issue:

```sh
$ ./vmctrl runsimple node1
```

This will start an instance of Fudge called node1. This node will not have any
networking abilities.

To enable networking, your user need to be allowed to create TAP interfaces on
your host machine. Typically sudo will allow you to do this.

```sh
$ sudo ./vmctrl run node1
```

Once you started one or more instances of Fudge you can bridge their networks
together with this command.

```sh
$ sudo ./vmctrl connect
```

Now each VM should be able to talk to eachother. In order for them to talk with
the outside world you need to set up NAT on your host machine. This can be done
by issuing the following command:

```sh
$ sudo ./vmctrl nat eth0
```

Replace eth0 with the interface that is connected to the internet. You only
need to run this command once unless you reboot your host machine.

## Community

There is a discord server:

Link: https://discord.gg/4nNq6Grhtz

There is also an irc channel:

Server: irc.freenode.net

Channel: #fudge

