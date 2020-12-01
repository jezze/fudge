# FUDGE OPERATING SYSTEM

http://www.fudgeos.com/

## OVERVIEW

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
leaving the higher level logic to be done in userspace. This is to not force a
design onto the user so they can choose to implement things differently.

## Building

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
# make install
```

The last command must be run as root.

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

Select:

```sh
Target options -> Target Architecture -> x86
```

By default the cross-compiler will be installed in ~/x-tools/x86-unknown-elf/.
You can change this if you want. Please refer to the crosstool-ng website for
more information on how to customize your setup.

Exit the configuration tool by pressing exit a few times. Before you leave the
last screen you will be asked if you want to save the settings. Press yes.

### Building the cross compiler

Start to build the cross-compiler:

```sh
$ ct-ng build
```

Depending on your system this may take some time, usually around 10-15 minutes.

### Get the Fudge source code

First you need to download the source code if you haven't done that already.

```sh
$ git clone git://github.com/jezze/fudge.git
$ cd fudge
```

### Set the PATH environment variable

In this example the cross-compiler was installed in
/home/foo/x-tools/x86-unknown-elf/. Change this to match your setup. If you
haven't made any major modifications you just need to change foo to match your
username instead.

```sh
$ export PATH=/home/foo/x-tools/x86-unknown-elf/bin:$PATH
```

The PATH modifications will be gone as soon as you leave the current session or
start another one. This means that you will need to reset it from time to time.

### Building Fudge

Finally we can build Fudge.

```sh
$ make ARCH=x86
```

You should now have a binary called fudge and an image called fudge.cpio in the
top directory of your source tree.

## Running in QEMU

There is a script you can use to start Fudge in QEMU called vmctrl.

```sh
# ./vmctrl run 1
```

This will start instance 1 of Fudge. Notice that this requires root because it
needs to set up some userspace networking. The 1 is a unique identifier for this
particular instance of Fudge. Valid number are between 1 and 254.

Once you started one or more instances of Fudge you can bridge their networks
together with this command.

```sh
# ./vmctrl connect
```

Now each VM should be able to talk to eachother. In order for them to talk with
the outside works you need to set up NAT on your host machine and as argument
you need to select the NIC that connects to the internet (like eth0 in this
example).

```sh
# ./vmctrl nat eth0
```

## COMMUNITY

Come talk to us on irc:

Server: irc.freenode.net
Channel: #fudge

