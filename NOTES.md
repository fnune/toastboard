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

### Running a recurring task with ESP8266_RTOS_SDK/FreeRTOS

Documentation on the ESP8266_RTOS_SDK does not feature any information about task creation. I guess this is because the underlying RTOS is FreeRTOS, so I'm going to try to find documentation for that.

Documentation link: https://www.freertos.org/a00125.html

Apparently I can create tasks with `xTaskCreate` or `xTaskCreateStatic`. If I want to use `xTaskCreateStatic`, I need to provide my own memory, so I need to pass more parameters, but I can allocate this memory at compile time. I'm going to go with the simpler `xTaskCreate`, which allocates from [the FreeRTOS heap](https://www.freertos.org/a00111.html).

The most important argument to `xTaskCreate` is the task function I want to run:

> Tasks are normally implemented as an infinite loop, and must never attempt to return or exit from their implementing function. Tasks can however delete themselves.

Another interesting argument is `usStackDepth`. It's in words, and the docs say a word is 32 bits in an ESP8266EX. One choice would be to find out the minimum stack size required for a task in FreeRTOS, but I researched a bit and found out that `memfault_platform_http_client_post_data` needs a stack of 192 bytes, or 48 words of an ESP8266EX. [Read below](#finding-out-the-stack-size-needed-by-a-function) for details.

To create a task that executes periodically, I just need to make it an infinite loop that calls `vTaskDelay` in it or `vTaskDelayUntil`. I'm going for the simpler `vTaskDelay`.

After creating the task, I get an crash loop due to a stack overflow on my new `task_upload_memfault_data`. Is my stack size not big enough?

#### Finding out the stack size needed by a function

I found this interesting article: [GNU Static Stack Usage Analysis](https://mcuoneclipse.com/2015/08/21/gnu-static-stack-usage-analysis/) by [Erich Styger](https://mcuoneclipse.com/author/mcuoneclipse/). I can pass `-fstack-usage` to `gcc` and get a nice `.su` file that gives me the stack usage of each function in bytes.

I found out I can pass append to `CFLAGS` for specific components, so I did so in `esp8266_sdk/memfault/component.mk`, and then found this file `memfault/memfault_platform_http_client.su`:

```
memfault_platform_http_client.c:178:5:memfault_platform_http_client_post_data	192	static
```

That's it! 192 bytes divided by my word size of 4 is exactly 48. I'll use 48 as a stack size for my task and eventually test to see if I can get by with less (I doubt it).

#### The stack size I calculated wasn't enough

I started by trying to allocate a stack of 48 words for my `task_upload_memfault_data` function, but that crashed my device due to bad allocations. I bluntly tried to increase it gradually until it worked, and it did! At 4096 words. I was off by a lot.

Could this be due to the fact that I measured stack usage for `memfault_platform_http_client_post_data` and not for my task's function `task_upload_memfault_data`? Let's build with `-fstack-usage` again, this time scouting for my task function instead. The number reported for `memfault_platform_http_client_post_data` was 192 bytes.

#### Finding out the size of a word manually in C

I remember in Rust this would be the size of `usize`. I guess in C I can go `sizeof(some_type)` to get this.

After some Googling, I found `size_t`, which varies depending on the address size of the processor, and is the type returned by `sizeof`. So let's try `sizeof(size_t)`.

```
ESP_LOGI(TAG, "Size of a word is %u bytes!", sizeof(size_t));
```

And then in `make monitor`:

```
I (532) TOASTBOARD: Size of a word is 4 bytes!
```

Four bytes are 32 bits. Thanks Toastboard!

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

### Trying to force a crash by flashing buggy code

Let's try adding an infinite loop in a `GET` request handler under `/crash`.

```
esp_err_t crash_get_handler(httpd_req_t *req)
{
    while (true) {
      // ...
    };

    const char* resp_str = (const char*) req->user_ctx;
    httpd_resp_send(req, resp_str, strlen(resp_str));

    return ESP_OK;
}

httpd_uri_t crash = {
    .uri       = "/crash",
    .method    = HTTP_GET,
    .handler   = crash_get_handler,
    .user_ctx  = "The application should crash before this shows up"
};
```

Calling `/crash` left my browser loading for about 30 (!) seconds before the device crashed. Here's the full output of the crash, from `make monitor`:

```
Saving Memfault Coredump!
Erasing Coredump Storage: 0x0 983040
coredump erase complete
Task watchdog got triggered.

Guru Meditation Error: Core  0 panic'ed (IllegalInstruction). Exception was unhandled.
Core 0 register dump:
PC      : 0x4026f3d4  PS      : 0x00000030  A0      : 0x40247d9c  A1      : 0x3fff57c0
0x4026f3d4: crash_get_handler at /home/fausto/Development/toastboard/src/main/main.c:54

0x40247d9c: httpd_uri at /home/fausto/.esp/ESP8266_RTOS_SDK/components/esp_http_server/src/httpd_uri.c:218

A2      : 0x40107cec  A3      : 0x4026f3d4  A4      : 0x00000002  A5      : 0x00000004
0x4026f3d4: crash_get_handler at /home/fausto/Development/toastboard/src/main/main.c:54

A6      : 0x00000073  A7      : 0x00000001  A8      : 0x00000001  A9      : 0x00000068
A10     : 0x00000068  A11     : 0x80808080  A12     : 0x40107c9c  A13     : 0x40107cec
A14     : 0x40107c9c  A15     : 0x40107cec  SAR     : 0x00000018  EXCCAUSE: 0x00000000

Backtrace: 0x4026f3d4:0x3fff57c0 0x40247d9c:0x3fff57c0 0x4024853b:0x3fff57e0 0x402485ed:0x3fff5860 0x402475df:0x3fff5880 0x40246f58:0x3fff5890 0x40246fc4:0x3fff58b0
0x4026f3d4: crash_get_handler at /home/fausto/Development/toastboard/src/main/main.c:54

0x40247d9c: httpd_uri at /home/fausto/.esp/ESP8266_RTOS_SDK/components/esp_http_server/src/httpd_uri.c:218

0x4024853b: httpd_parse_req at /home/fausto/.esp/ESP8266_RTOS_SDK/components/esp_http_server/src/httpd_parse.c:527

0x402485ed: httpd_req_new at /home/fausto/.esp/ESP8266_RTOS_SDK/components/esp_http_server/src/httpd_parse.c:594

0x402475df: httpd_sess_process at /home/fausto/.esp/ESP8266_RTOS_SDK/components/esp_http_server/src/httpd_sess.c:327

0x40246f58: httpd_server at /home/fausto/.esp/ESP8266_RTOS_SDK/components/esp_http_server/src/httpd_main.c:113

0x40246fc4: httpd_thread at /home/fausto/.esp/ESP8266_RTOS_SDK/components/esp_http_server/src/httpd_main.c:113

@R%dPMT%w@Y)Q
OT5V'I
T       )dE     5Y
NZA     I^NTR
@\Y!NZA I^{YP1g@\@Y
[)!E   pL@LYYX)IIPKVqRA I       YЛ     pL@PWK   V       ZQHYYV[)!E     pL@Y^ZYY
                         1eʉE  xO@Y     J5)aEEAJA%Y     J)Q
AEaE   xL@Y     J-I
%ʉE    xO@YZAFPYM)!E   `M@YI9V5ԻT)dVEEAJAH%Y=eZ)R
                                                 E
                                                  E    h%Y-JP-G

                                                               E       h%Y1iGQP)PNP-E  h%Y--HPPG
-AKRMYV@Q                                                                                       YAJAH^-G-HPP-YAJAH^w\PRZANPYEEAJAHq
         E     h)YVYP1DT
                        PTg
                           TWp)ʉE      hM)YVY1DT
                                                P%=DT
                                                     T5қ
                                                        @)ʉE   h)YVYRP#PR
5P@H)IJ)!E     hN)YVRP%=DT
                          Tg
)IJ)!E h                    P!)IJ)!E   h)YVYPRPT5eHP@
        %YYP-^1qQHT-TRYI (411) system_api: Base MAC address is not set, read default base MAC address from EFUSE
```

It looks like the system gave me the backtrace and the exact location of my crash: the body of the `crash_get_handler`. What a surprise.

On the Memfault application nothing's showing up yet. I guess it's because my device doesn't upload data unless I tell it to, using the `GET /upload` route.

After I upload the data in the device, I get this in the issues view:

```
Unprocessed traces exist due to missing symbol file(s):

Upload 1.0.0+5a05af (toastboard-firmware)
```

Cool! Uploading that now. I'm such a noob that I don't even know what I'm supposed to upload. I think it's `build/toastboard.elf`, though.

Let's try crashing the same away again to test issue deduplication. While my device takes some time to decide to crash through the infinite loop, I'm thinking of two questions:

1. Why does it take so long for it to crash? I'd have guessed the timeout is shorter. Although it's an infinite loop, shouldn't it still be cancelable by some interrupt or whatever mechanism the RTOS has?
2. Why is it showing up as a `Hard Fault` instead of a `Watchdog` issue? Wasn't the device restarted by the watchdog?

Looking around in `make menuconfig`, I found the answer to (1): `Task Watchdog timeout period (seconds) (26.2144s)` under `ESP8266-specific`. I was pretty close with my 30 seconds guess!

I think I can answer (2): the issue is a `Hard Fault`. The reboot event should be have the watchdog as a cause. However, it shows up as `Unspecified`. I guess I have something else to integrate.

Issue deduplication seems to be working fine. Here's what the Queue Status widget shows:

```
a few seconds ago	Attached to Issue #38075
26 minutes ago	Attached to Issue #38075
```

Both were attached to issue `38075`.

### Integrating reboot reasons

Documentation link: https://docs.memfault.com/docs/embedded/reboot-reason-tracking

I guess my reboot is showing up as having an `Unspecified` reason because I haven't completed integration. Let's continue with that.

The guide mentions I should `#include "memfault/core/reboot_tracking.h"`, but I confirmed that's already included with `"memfault/components.h"`.

There are two things I need to figure out from my chip and use in the initialization code:

1. A pointer to the reboot reason register.
2. Additional information about the reboot reason, mapped to a `MfltResetReason`.

As for (2), the ESP8266_RTOS_SDK documentation points to a function [`esp_reset_reason`](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/release-v3.3/api-reference/system/system.html?highlight=reset#_CPPv416esp_reset_reasonv) that returns a value of type [`esp_reset_reason_t`](https://docs.espressif.com/projects/esp8266-rtos-sdk/en/release-v3.3/api-reference/system/system.html?highlight=reset#_CPPv418esp_reset_reason_t). I need to figure out how to map from `esp_reset_reason_t` to `MfltResetReason`.

As for (1), I can't find information in the documentation indicating how to access that register. There's only the functions described above. Looking at  the RTOS source code, I found this: `RTC_RESET_HW_CAUSE_REG`. I'm wondering if that's defined. To find that out, I'm going to try to compile with that and `kMfltRebootReason_Unknown` for now.

```
error: 'RTC_RESET_HW_CAUSE_REG' undeclared
```

Nope! However, I found that `RTC_RESET_HW_CAUSE_REG` points to a register `RTC_STATE1`, which I can access if I `#include "esp8266/rtc_register.h"`. This feels really hacky, but the code now compiles!

Reboot reasons still show up as `Unspecified` and `Unexpected Reset`. Time to continue with the integration: mapping `esp_reset_reason_t` to `MfltResetReason` and passing it to the Memfault SDK.

I built the mapper and passed it to `memfault_reboot_tracking_boot`, and everything compiles just fine, but the reboots in the app are still `Unspecified`. There's more integration steps to take, so I probably need to continue.

I needed to dedicate some storage space for events and pass it to Memfault. Once that was done, I tortured the little thing once more with the `GET /crash` endpoint, and voila! There's a reason for my reboot on the Memfault application: `Software Watchdog`.

This is a bit weird because the logs from `make monitor` show this:

```
I (503) mflt: ESP Reset Cause 0x7
I (509) mflt: Reset Causes:
I (514) mflt:  Hardware Watchdog
```

Maybe I got the mappings wrong!

```
/* Reset (software or hardware) due to interrupt watchdog. */
case ESP_RST_INT_WDT: return kMfltRebootReason_HardwareWatchdog;
/* Reset due to task watchdog. */
case ESP_RST_TASK_WDT: return kMfltRebootReason_SoftwareWatchdog;
/* Reset due to other watchdogs. */
case ESP_RST_WDT: return kMfltRebootReason_SoftwareWatchdog;
```

Reset cause `0x7` corresponds to "other watchdogs", so I'm going to map that to `HardwareWatchdog` instead, since that's what the logs say. The `Hardware Watchdog` log from the Memfault SDK was there long before I added my mapping, so that suggests to me that they knew the reset reason all along. Why did I have to implement the mapping, then?
