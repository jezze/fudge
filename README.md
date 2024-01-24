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
be dropped. By default the system has callbacks for MAIN, TERM and REDIRECT
messages but these can be overridden. It is possible to poll messages manually
from the channel, either from a certain sender or of a certain type. Anything
that falls outside of that will be automatically dispatched to the default
registered callbacks. It is also possible to dispatch a message manually.

## Building the cross-compiler (crosstool-ng)

In order to compile Fudge for x86 we need to have a cross-compiler. Building a
cross-compiler can be a tedious task but luckily there is a tool called
crosstool-ng that automates the entire process for us.

Download and install crosstool-ng. The recommended version is 1.26.0.

```sh
$ wget http://crosstool-ng.org/download/crosstool-ng/crosstool-ng-1.26.0.tar.bz2
$ tar xjf crosstool-ng-1.26.0.tar.bz2
$ cd crosstool-ng-1.26.0
$ ./configure
$ make
$ sudo make install
```

Create a temporary folder in your home directory and cd to it:

```sh
$ mkdir ~/temp
$ cd ~/temp
```

Configure our new cross-compiler build by running:

```sh
$ ct-ng menuconfig
```

This will start the configuration tool. Set the target architecture to x86:

```sh
Target options -> Target Architecture -> x86
```

By default the cross-compiler will be installed in ~/x-tools/i386-unknown-elf/.
You can change this if you want. Please refer to the crosstool-ng website for
more information on how to customize your setup.

Exit the configuration tool by pressing exit a few times. Before you leave the
last screen you will be asked if you want to save the settings. Press yes.

Start building the cross-compiler:

```sh
$ ct-ng build
```

Depending on your system this may take some time, usually around 10-15 minutes.

## Building Fudge

This step requires that you to have the cross-compiler built.

First you need to download the source code if you haven't done that already.

```sh
$ git clone git://github.com/jezze/fudge.git
$ cd fudge
```

Now we need to make sure our system can find the new cross-compiler that we
created previously. This is done by adding the path to the compiler to the
$PATH environment variable on your host. Unless you changed the install path,
this should work:

```sh
$ export PATH=/home/$(whoami)/x-tools/i386-unknown-elf/bin:$PATH
```

The changes to $PATH will disappear as soon as you leave the current session or
start another one so you will need to set it again from time to time.

Finally we can build Fudge.

```sh
$ make
```

You should now have a binary called fudge and a ramdisk image called fudge.cpio
in the top directory of your source tree.

## Running Fudge in QEMU with vmctrl

There is a script call vmctrl that you can use to run Fudge as a virtual
machine in QEMU.

```sh
$ ./vmctrl runsimple node1
```

This will start an instance of Fudge called node1. This node will not have any
networking abilities however. To enable networking, your user need to be
allowed to create TAP interfaces on your host machine. Typically using sudo
will allow you to do this.

```sh
$ sudo ./vmctrl run node1
```

Once you started one or more instances of Fudge you can bridge their networks
together with this command.

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

## Community

There is a discord server:

Link: https://discord.gg/4nNq6Grhtz

There is also an irc channel:

Server: irc.libera.chat

Channel: #fudge

