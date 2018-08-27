# GVM -- the golfing 8bit virtual machine

This is an attempt to create an 8bit virtual machine inspired by the famous
6502 chip that's better suitable for "code golfing" than the original. It
isn't intended to compete with golfing languages though.

### Building

Get the source with:

    git clone https://github.com/zirias/gvm --recurse-submodules

If you already cloned the source without submodules, the following command
will fix it:

    git submodule update --init --recursive

Then, build `gvm` by typing

    make

or

    make strip

for a stripped binary. The build system uses **GNU make**, so if you're on a
system where the default `make` utility isn't GNU make (like e.g. FreeBSD),
you have to type `gmake` instead. For more information, see [zimk](
https://github.com/zirias/zimk).

### Usage

To be done. For now, type `gvm -h` to get a help message.

### Programming gvm

To be done. For now, see [opcode.h](src/opcode.h) to get a general idea --
most of these opcodes are inspired by the 6502.

