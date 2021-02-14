# Notes

I'm a web developer, so this is all new to me. Here are my notes.

## ESP8266_RTOS_SDK

### Toolchain

Documentation link: https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/linux-setup.html

I'm surprised that I need to download tarfiles.

- I want tooling that helps me achieve reproducible builds.
- The Espressif docs encourage me to install things in `~/esp` and then add them to my path. What if I lose access to this version of the toolchain and I want to run this in the future?
- There seems to be no unifying tool, I guess the ecosystem for each board is a different world.

---

A first `ls` in the binaries offered by the toolchain gives me this:

```bash
 ~/Development/toastboard => ls ~/.esp/xtensa-lx106-elf/bin
xtensa-lx106-elf-addr2line  xtensa-lx106-elf-c++filt       xtensa-lx106-elf-gcc         xtensa-lx106-elf-gcov       xtensa-lx106-elf-ld       xtensa-lx106-elf-ranlib
xtensa-lx106-elf-ar         xtensa-lx106-elf-cpp           xtensa-lx106-elf-gcc-8.4.0   xtensa-lx106-elf-gcov-dump  xtensa-lx106-elf-ld.bfd   xtensa-lx106-elf-readelf
xtensa-lx106-elf-as         xtensa-lx106-elf-ct-ng.config  xtensa-lx106-elf-gcc-ar      xtensa-lx106-elf-gcov-tool  xtensa-lx106-elf-nm       xtensa-lx106-elf-size
xtensa-lx106-elf-c++        xtensa-lx106-elf-elfedit       xtensa-lx106-elf-gcc-nm      xtensa-lx106-elf-gdb        xtensa-lx106-elf-objcopy  xtensa-lx106-elf-strings
xtensa-lx106-elf-cc         xtensa-lx106-elf-g++           xtensa-lx106-elf-gcc-ranlib  xtensa-lx106-elf-gprof      xtensa-lx106-elf-objdump  xtensa-lx106-elf-strip
```

I can only guess what some of those things are for. I haven't read the full documentation yet. Here are my **guesses**:

- `xtensa-lx106` is the architecture, just like my computer's is `x86_64`?
- `xtensa-lx106-elf` specifies the ELF format these utilities deal with. I know about ELF files, but I'm not sure if they can be in different formats or not, or which part is different.
- I assume `*-gdb` is an `xtensa-lx106`-specific build of GDB. It would be interesting to see if there's a "default" version of GDB.
- `*-gprof` is for profiling.
- `*-objdump` is to show me the content of a built executable in a more human-readable way?
- `*-strip`, `*-strings`, `*-ranlib` all sound like libs I can use in my programs.
- `*-gcov` is a test coverage tool?
- `*-gcc` is the compiler! I'm pretty sure about that one! Is `*-cc` just an alias?
- `*-strip` is for removing or renaming symbols?

Some of these tools are already available in my `PATH` but not specifically for `xtensa-lx106-elf`, I guess because I've installed `build-essential`: `gcc`, `strip`, `objdump`, `objcopy`... what do these binaries work on? `x86_64` Linux?

---

The "supported" IDE is Eclipse, but the docs start with this:

> We suggest building a project from the command line first, to get a feel for how that process works. You also need to use the command line to configure your ESP8266_RTOS_SDK project (via make menuconfig), this is not currently supported inside Eclipse.

That means I'll get introduced to interacting with the project via command line, so that's good.

### Installing ESP8266_RTOS_SDK

Documentation link: https://docs.espressif.com/projects/esp8266-rtos-sdk/en/latest/get-started/index.html#get-esp8266-rtos-sdk

Once again, the documentation suggests I just dump the whole RTOS into `~/esp`. Should I be using submodules instead? Maybe for the toolchain, too. The next developer that works on this is inevitably going to clone `master` at a different commit. Let's just follow along for now.

> The toolchain programs access ESP8266_RTOS_SDK using IDF_PATH environment variable. This variable should be set up on your PC, otherwise projects will not build. Setting may be done manually, each time PC is restarted. Another option is to set up it permanently by defining IDF_PATH in user profile.

I suppose this means I need `export IDF_PATH=~/esp/ESP8266_RTOS_SDK` in my `.zshrc`.

Next up, I'm installing some Python packages globally. I think if I wanted to make these builds reproducible I'd have to go through a lot more steps than those described in the documentation. Or maybe these tools just don't change so much?

### Running the `hello_world` example project

At first, I was surprised that `make menuconfig` is what I'm supposed to run here, because that target is not in the `Makefile`, but these are its contents:

```
PROJECT_NAME := hello-world

include $(IDF_PATH)/make/project.mk
```

I didn't know you can include `Makefile`s from other `Makefile`s.

---

Running `make menuconfig` fails because it expects `python` in `PATH`, but I have `python3`. I also installed the required libraries using `python3`, so I'm going to install `python` (which in Ubuntu means 2.7) and reinstall the dependencies using `python`. I think on Arch `python` points to Python 3.

After doing that, I get:

```
pkg_resources cannot be imported probably because the pip package is not installed and/or using a legacy Python interpreter. Please refer to the Get Started section of the ESP-IDF Programming Guide for setting up the required packages.
```

Apparently `pip` points to `python3-pip` in my system, so now I have to find out whether this works after installing `python2-pip` (I'm guessing that's the name) or if I do need everything on Python 3 and I need to solve the `python` vs `python3` name problem in some other way.

---

I ended up using `alias python=python3` and running `sudo apt install python-is-python3` because the SDK depends on `/usr/bin/env python`, making my alias useless. I guess they should have just fixed that to `/usr/bin/env python3`.
