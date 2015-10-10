# MVP: Multi-core Virtual Platform

## Overview

MVP is a multi-core whole system simulation virtual platform based on SystemC. It has:

* ISS (instruction set simulator) supports ARMv5/v6/v6K/v7-A instruction set
* Essential peripheral modules for Linux booting: interrupt controller, system controller, timer, uart 

It is also the base platform for the [CASL Hypervisor](https://github.com/ufoderek/casl_hypervisor).

## System Requirement

1. Modern Mac OS X or Linux distributions (tested on OS X El Capitan)
2. SystemC 2.3.1 (with TLM)
3. Boost 1.58

## Build

1. Download and build SystemC from http://accellera.org.
2. Download and install the Boost Library or get it via Homebrew.
3. Edit Defs.mk to include proper pathes for SystemC before compiling.

```
    $ make pb -j4    # build for Realview Platform Baseboard with ARMv7-A ISS

    $ make vb -j4    # build for CASLab Virtual Platform with ARMv7-A ISS and Virtualization Extensions
```

Commands below are not functional yet.

```
    $ make v -j4     # build for Versatile Platform Baseboard with ARMv5 ISS

    $ make r -j4     # build for Realview Emulation Baseboard with ARMv5 ISS

    $ make rr -j4    # build for Realview Emulation Baseboard with ARMv6 ISS

    $ make rmp -j4   # build for Realview Emulation Baseboard with ARMv6K multi-core ISS
```

## Implementation Detial

### Versatile Platform Baseboard with ARMv5 ISS

Both the Versatile PB and the ARMv5 ISS implementations are based on Sing-Ying Lee's work traced back to 2010.

It can boot up a bare Linux kernel with a simple initial RAM disk. Only instructions being used during Linux boot were implemented.

For implementations, refer to mvp/armv5 and mvp/versatilepb_top.

### Realview Emulation Baseboard with ARMv5, ARMv6 and ARMv6K ISS

The single-core ARMv6 ISS and multi-core ARMv6K ISS extends the ARMv5 ISS using C++ Inheritance.

This platform can boot up a multi-core (up to 4) Linux kernel with a simple initial RAM disk. Only instructions being used during Linux boot were implemented.

For implementations, refer to mvp/arm6, mvp/armv6k and mvp/realvieweb_top.

### Realview Platform Baseboard with ARMv7-A ISS

The ARMv7-A ISS is a complete re-write due to the huge differences between ARMv6 and ARMv7 instructions sets.

The ARM Virtualization Extensions is implemented in the v7 ISS, but the multi-core support is not.

It can boot up a bare Linux kernel with a simple initial RAM disk. Only instructions being used during Linux boot were implemented.

For implementations, refer to mvp/arm7a and mvp/realviewpb_top.

### CASLab Virtual Platform with ARMv7-A ISS and Virtualization Extensions

This is a completely virtual platform which is used to demonstrate the ARM-based [CASL Hypervisor](https://github.com/ufoderek/casl_hypervisor). It contains necessary peripherals to support up to four guest OSes for the [CASL Hypervisor](https://github.com/ufoderek/meteor).

For implementations, refer to mvp/arm7a and mvp/meteorvb_top.
