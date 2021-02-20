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

> We suggest building a project from the command line first, to get a feel for how that process works. You also need to use the command line to configure your ESP8266_RTOS_SDK project (via `make menuconfig`), this is not currently supported inside Eclipse.

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

---

The docs ask me to do this:

> You are almost there. To be able to proceed further, connect ESP8266 board to PC, check under what serial port the board is visible and verify if serial communication works. Note the port number, as it will be required in the next step.

As if I knew how to do that! Time to DuckDuckGo.

It seems like the Internet has settled on this command:

```bash
~/Development/toastboard [2] => sudo dmesg | grep tty
[    0.183219] printk: console [tty0] enabled
[    0.784336] 0000:00:16.3: ttyS4 at I/O 0x3060 (irq = 19, base_baud = 115200) is a 16550A
[  594.954203] usb 1-1: cp210x converter now attached to ttyUSB0
[  679.763000] cp210x ttyUSB0: cp210x converter now disconnected from ttyUSB0
```

At first, I thought my device was connected to `/dev/ttyS4` because `sudo dmesg | grep tty` showed it alongside a baud rate, but perhaps that's the default for the port? As it turns out, my device is on `/dev/ttyUSB0`.

Part one done: my device is on `/dev/ttyUSB0`.

As for verifying that serial communication works: I've read mentions of `minicom`, so I'm going to try it out. After some configuration, I get a dot each second, which is the serial output a previous Arduino program I wrote outputs. It's working! Here's the command I ran:

```bash
sudo minicom -b 115200 -o -D /dev/ttyUSB0
```

I run `make menuconfig`. The serial port is set to `/dev/ttyUSB0`. The generated configuration looks really specific to my laptop, so I'm going to put it in `.gitignore`. It seems like other developers would have to just run `make menuconfig` on their machines, too.

The output of `make flash` looked promising until I got a `Permission denied: /dev/ttyUSB0` error. I suppose I can run `sudo make flash`, but that seems a bit excessive.

```
~/Development/toastboard/hello_world [2] => ls -l /dev/ttyUSB0
crw-rw---- 1 root dialout 188, 0 Feb 14 21:09 /dev/ttyUSB0
```

If I add myself to the `dialout` group, I should have access to my device. Thanks SO.

```
sudo usermod -a -G dialout $USER
```

After a reboot (surprisingly, logging out and then logging back in didn't work), I could flash the RTOS onto the device. It's now blinking its LED very infrequently.

The serial communication output is interesting:

```
rnn||bll
```

It seems to then print some more `rnn||bll` lines, then clears itself and restarts when the LED blinks.

---

Reading the code in [`hello_world_main.c`](hello_world/main/hello_world_main.c) I can't find much correlation between what I get through the serial port and the code. Perhaps only that there's a loop that resets the device every ten seconds, and that's when the LED blinks:

```c
for (int i = 10; i >= 0; i--) {
    printf("Restarting in %d seconds...\n", i);
    vTaskDelay(1000 / portTICK_PERIOD_MS);
}
```

I need to read into the `printf` function. Maybe there's a decoding problem, or the baud rate is wrong. Time to continue reading the docs!

A `make monitor` target is included. Running it produces serial output at the same rate as the `rnn||bll` before but this time it's completely readable, and matches what's in the source code (go figure). I'm wondering how I could produce the same results using `minicom`. There's a `74880` number in the output of `make monitor` that looks suspiciously like a baud rate. I was connecting with `115200`.

I thought the baud rate was the problem, but running `minicom -b 74880 -o -D /dev/ttyUSB0` still gave me the `rnn||bll` output.

Apparently, you can specify baud rates like this: `make flash ESPBAUD=9600`, so I'm trying to see if the `minicom` output changes with this. Flashing at `9600` is so slow! Some minutes later, I run `minicom -b 9600 -o -D /dev/ttyUSB0`, and I get slightly different results, but still gibberish. More or less this:

```
ɏɏɏɏ
ɏɏɏɏɏɏɏɏɏɏɏɏ
GGq4O.
```

Time to read the source code to figure out what's up. The output of `make monitor` includes some interesting information:

- The script that runs is `idf_monitor`, and its source code is in `$IDF_PATH/tools/idf_monitor.py`.
- I can modify that file and change the output of `make monitor`.
- This helped me find out that the default baud rate is `74880`.

Running `screen /dev/ttyUSB0 74880` produces gibberish, too:

```
GGq4O.���ɏ�ɏQ�0�~?�4�!�S{�O�:9�O�:9�COAa�$\���S��4�MS��F�����.P�r|Vz4E^V8���M�0�zx��#A�AVZ��#�pRZR�rRRZ�E��&��,CORRV��'K�EEZG�Ň�EERGG���MK�EERG��'K�EEZG��#�G�����Gq���4O.�ɏ�ɏ��ɏ�ɏ��ɏ�ɏ��ɏ�ɏ��ɏ�ɏQ�0�~?�4�!�S{�O�:9�O�:9�COAa�$L���S��4�MS��F�����.P�r|Vz4E^V8���M�0�zx��#A�AVZ��#�pRZR�rRRZ�E��&��,CORRV��'K�EEZG�Ň�EERGG���MK�EER
```

Rebuilding with baud rate 115200 makes `screen /dev/ttyUSB0 115200` produce output more similar to the original output from `minicom`:

```
GGq4O.���ɏ�ɏQ�0�~?�4�!�S{�O�:9�O�:9�COAa�$\���S��4�MS��F�����.P�r|Vz4E^V8���M�0�zx��#A�AVZ��#�pRZR�rRRZ�E��&��,CORRV��'K�EEZG�Ň�EERGG���MK�EERG��'K�EEZG��#�G�����Gq���4O.�ɏ�ɏ��ɏ�ɏ��ɏ�ɏ��ɏ�ɏ��ɏ�ɏQ�0�~?�4�!�S{�O�:9�O�:9�COAa�$L���S��4�MS��F�����.P�r|Vz4E^V8���M�0�zx��#A�AVZ��#�pRZR�rRRZ�E��&��,CORRV��'K�EEZG�Ň�EERGG���MK�EER��r�b�nn�lnnl��r�nn�||�bbl��r�b�nn�lnn��r�nn�||�bbl��r�b�nn�lnn��r�nn�||�bbl��r�b�nn�lnn��r�nn�||�bbl��r�b�nn�lnnl�r�nn�||�bbl��r�b�nn�lnn�r�nn�||�bbl
```

At this point I'm going to give up and just use `make monitor`, and figure this out later with the help of someone more experienced.

### A foray into Linux specifics

Here's what I know about `dmesg`: it prints logs from the kernel. And here's all my loose bits of unchecked knowledge about `tty`:

- It stands for TeleTypewriter.
- In Linux, I can go to a different `tty` by using `CTRL+ALT+F#`.
- Some of the `#`s are used by specific processes. In my case, `CTRL+ALT+F1` takes me to GDM (which shows my log-in screen), and `CTRL+ALT+F2` takes me back to my current Xorg session.
- I've used `CTRL+ALT+F#` with numbers other than 1 and 2 to get me out of problems with me Xorg session.
- I suppose that means processes are tied to `tty`s, or `tty`s are somehow pointers to processes.

Time to strengthen this knowledge.

```bash
man tty
```

> `tty` - print the file name of the terminal connected to standard input

```bash
~/Development/toastboard/hello_world => tty
/dev/pts/3
```

I have three terminals open, and that one was number 3 apparently.

A TTY is transitively [a software emulation of a TeleTypewriter](https://www.howtogeek.com/428174/what-is-a-tty-on-linux-and-how-to-use-the-tty-command/), or a pseudoterminal. In Linux, there's the concept of a pseudoterminal master and pseudoterminal slaves.

> The file /dev/ptmx is a character file with major number 5 and minor number 2, usually of mode 0666 and owner.group of root.root. It is used to create a pseudoterminal master and slave pair. When a process opens /dev/ptmx, it gets a file descriptor for a pseudoterminal master (PTM), and a pseudoterminal slave (PTS) device is created in the /dev/pts directory. Each file descriptor obtained by opening /dev/ptmx is an independent PTM with its own associated PTS, whose path can be found by passing the descriptor to ptsname(3).

I suppose this is what terminal emulators such as Kitty do when they open: they get assigned a file under `/dev/pts`. It must also be what `tmux` does when I open a new pane or window.

### Playing with `minicom`

On a first attempt to verify that serial communication to my development board works, I did this:

```
 ~/Development/toastboard/hello_world => minicom -p /dev/ttyS4
minicom: argument to -p must be a pty
```

A `pty` is a pseudoterminal. I guess if I open a new terminal, it'll be assigned `/dev/pts/4` and then I can communicate to it? I'm curious to try it out.

Open a new terminal and then on terminal `/dev/pts/3`:

```
minicom -p /dev/pts/4
```

Type some stuff... and it appears on my newest terminal! Cool! Trying to type on `/dev/pts/4` produces the same characters on `/dev/pts/3`, but some get dropped. I'm curious to find out why, but let's get back to the main course.

## The Memfault SDK

Documentation link: https://docs.memfault.com/docs/embedded/esp8266-rtos-sdk-guide/

I'm starting by getting some application code in place. It's just going to be an HTTP server with simple routes. Then, I'll try to integrate the Memfault SDK on top.

Ran into this while compiling:

```
In file included from /home/fausto/Development/toastboard/src/components/memfault_port/memfault-firmware-sdk/components/include/memfault/components.h:20,
                 from /home/fausto/Development/toastboard/src/main/main.c:17:
/home/fausto/Development/toastboard/src/components/memfault_port/memfault-firmware-sdk/components/include/memfault/config.h:30:39: fatal error: memfault_platform_config.h: No such file or directory
 #define MEMFAULT_PLATFORM_CONFIG_FILE "memfault_platform_config.h"
                                       ^~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

I later found this from the `CHANGES.md` file in the Memfault SDK:

```md
- You must create the file `memfault_platform_config.h` and add it to your
  include path. This file can be used in place of compiler defines to tune the
  SDK configurations settings.
```

After solving that by creating `src/main/include/memfault_platform_config.h`, there was another missing header file:

```
/home/fausto/Development/toastboard/src/components/memfault_port/memfault-firmware-sdk/ports/esp8266_sdk/memfault/esp_reboot_tracking.c:14:10: fatal error: internal/esp_system_internal.h: No such file or directory
 #include "internal/esp_system_internal.h"
          ^~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
```

I got a bunch of compiler errors because I was on the `master` branch of the SDK and the Memfault integration supports only v3.3. The changes from `master` to v3.3 are significant in the HTTP server example, so I'm updating my application code.
The next compiler error was a missing header file `esp_console.h` expected in `memfault_cli.c`. I could solve it by adding this to `src/sdkconfig`:

```
CONFIG_USING_ESP_CONSOLE=y
```

Et voila! It has built! Time to get it flashed onto the board.

### Reboot loop after integrating the SDK

There are some useful logs:

```
I (492) system_api: Base MAC address is not set, read default base MAC address from EFUSE
Guru Meditation Error: Core  0 panic'ed (LoadProhibited). Exception was unhandled.
```

And:

```
E (463) mflt: Coredumps enabled but no storage partition found!
```

I'm not sure which one is causing the reboots, so I'm going to fix the MAC address problem first. It should be a matter of using a different method that doesn't get the "base" MAC address but the one from EFUSE.

---

I gave up trying to fix the MAC address problem because no matter what I picked, it would crash. I hereby name this device `"the-one-and-only"`.

We're left with:

```
E (463) mflt: Coredumps enabled but no storage partition found!
```

It's a bit weird because I'm still not at the step where I can [add coredumps integration](https://docs.memfault.com/docs/embedded/coredumps/).

Either way: I can print the partition table of the device with `make partition_table`:

```
# Espressif ESP32 Partition Table
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  24K,
phy_init, data, phy,     0xf000,  4K,
factory,  app,  factory, 0x10000, 960K,
```

[My chip](https://www.amazon.de/-/en/gp/product/B08BTXCZC1/ref=ppx_yo_dt_b_search_asin_title?ie=UTF8&psc=1) supposedly has 4MB of storage, and that's roughly 1MB in the partition table, so I guess I could flash something else onto it. For now, I'm going to go with this.

Now I need to:

1. Identify which of those is the storage partition.
2. Figure out how to tell Memfault to use that.

The [ESP8266_RTOS_SDK documentation on partition tables](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/release-v3.3/api-guides/partition-tables.html?highlight=storage) says NVS stands for non-volatile storage. I guess that's what we want. That means I want the partition starting at `0x9000` that's 24K in size.

I'm going to look for a way to do (2) in `make menuconfig`.

The Memfault-specific configuration has an option "Enable saving a coredump to flash storage (OTA slot)". The other choice is "User defined coredump storage region". I guess I could pick the second, but there's a default partition table that includes an OTA slot from the SDK, so I'm going to use that instead of the NVS partition.

The configuration changed like so:

```diff
- CONFIG_PARTITION_TABLE_SINGLE_APP=y
+ # CONFIG_PARTITION_TABLE_SINGLE_APP is not set
- # CONFIG_PARTITION_TABLE_TWO_OTA is not set
+ CONFIG_PARTITION_TABLE_TWO_OTA=y
```

I'm going to add that to the defaults file because it seems critical to the project. I also changed how I'm getting my WiFi SSID and password. Time to flash it again!

### The reboot loop continues

But now it's different! There are no more errors from Memfault, and I get a reassuring message:

```
I (499) mflt: Coredumps will be saved at 0x110000 (983040B)
```

This means my new partition table is being used correctly.

To begin debugging the issue, I removed the `memfault_platform_boot()` call. Now, the device works correctly and does what it's supposed to do (not much). I can look into my Memfault-related files to find the issue.

I'm going to hardcode the hardware version instead of trying to get it from `chip_info`, since I'm suspicious of my C.

After removing that, the device is stable! I just don't know C.

### No data

As I don't know how to set up a periodic task on FreeRTOS, I didn't add the `memfault_esp_port_http_client_post_data()` line.

My device exposes an HTTP server, so I decided to create a `GET /upload` route that will upload anything saved up to Memfault.

I open `192.168.0.24:80/upload` on a browser and...

```
D (30030) mflt: Posting Memfault Data
D (30339) mflt: Posting Memfault Data Complete!
I (30342) mflt: Result: 0
```

My device now exists on Memfault!

```
the-one-and-only	1.0.0+4bb166	ESP8266EX_CH340C	a minute ago
```

### The ESP-IDF console

The Memfault docs mention a `crash` CLI command. I'm not sure what this is. I tried to find some information about this, thinking it's related to the `CONFIG_USING_ESP_CONSOLE=y` flag I passed in for compilation. The ESP8266 documentation doesn't mention the `console` component or the `crash` CLI command.

ESP-IDF documentation mentions this:

> ESP-IDF provides console component, which includes building blocks needed to develop an interactive console over serial port.

I guess I'm supposed to build the interactivity into a new target? Maybe I'm supposed to test with the `console` example provided with the RTOS.

I'm blocked at this step because the next steps depend on this: the crash data I could get from the `crash` CLI would be uploaded with the `memfault` CLI, and then the web application would warn me there's a missing symbols file for the data, and I could then upload the symbols file and get issues from it.

I guess I need to find a way around the problem if I can't find out what this `crash` CLI command is.

