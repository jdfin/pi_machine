# Pi Machine

Pi Machine prints pi, for as long as you let it run. It was inspired by a couple of projects covered at [Hackaday](https://hackaday.com/): the [Pi Spigot](https://hackaday.com/2021/04/06/raspberry-pi-spigot-puts-digits-of-pi-on-tap/) and [Happy Pi Day 2022](https://hackaday.com/2022/03/15/simple-arduino-build-lets-you-keep-an-eye-on-pi/). The latter's [project page](https://www.linkedin.com/pulse/happy-%25CF%2580-day-2022-cristiano-monteiro) describes how it uses [Xavier Gourdon's](http://numbers.computation.free.fr/Constants/constants.html) [nth-digit algorithm](http://numbers.computation.free.fr/Constants/Algorithms/nthdigit.html) to compute and display digits directly (i.e. compute the nth digit without (necessarily) having computed all the digits before it).

The difference between those and this Pi Machine is that this one prints digits on a thermal printer, and contains logic and hardware (paper-change, power-fail) that in theory lets it run forever. The software herein can be configured to run without any special hardware (other than an Arduino-capable board), but in that case it will just print digits to the serial console.

<img src="/assets/top-front-left.jpg" width="300">

The complete (as-built) Pi Machine uses:
* [Adafruit Feather M4 Express](https://www.adafruit.com/product/3857). Minimizing the hardware (how slow can you go) might point to an M0 board, but I had some of the M4 Express "in stock". I first tried on a [DF Robot Beetle](https://www.dfrobot.com/product-1075.html), but that doesn't (easily, out-of-the-box) support 64-bit doubles, and 32-bit doubles lead to a failure at about digit 768, possibly due to the six nines in a row there. It works on an M0 ([Seeeduino XIAO](https://www.seeedstudio.com/Seeeduino-XIAO-Arduino-Microcontroller-SAMD21-Cortex-M0+-p-4426.html) or [Adafruit QT Py - SAMD21](https://www.adafruit.com/product/4600), but I wanted a battery charger, which led me to the [Feather](https://www.adafruit.com/category/943). The [M0 Express](https://www.adafruit.com/product/3403) would be the "lowest-end" hardware, but I had a few of the M4 Express, so the Pi Machine is twice as fast as it could have been. It also runs on a [PJRC Teensy 4.0](https://www.pjrc.com/store/teensy40.html), but 600 MHz and hardware floating point kinda spoils the fun. And I wanted the charger.
* [Thermal Printer](https://www.adafruit.com/product/597). This link is to the printer at Adafruit, but I think you can get the same printer from a number of places. I think the firmware can (will) be different depending on where you get it - I got one of the Adafruit printers via Mouser, and one from Adafruit directly, and they are different in some user-apparent ways. I ended up not using the Adafruit library for it, just because I was innit enough trying to figure it out that I used my own stuff (so the library in here is not suitable for general purpose use). One of the smaller printers would work (after dealing with firmware differences), but the larger paper capacity of the "Mini" seems nice.
* [LiPo](https://www.adafruit.com/product/4236). The purpose of the LiPo is to let you unplug the Pi Machine to move it and not have it start over. I briefly tried making it so it would run from the battery, but it's hard getting enough power to the thermal printer and clean power to the microprocessor such that light printing and brownouts are not an issue. So it charges while plugged in, and when Pi Machine detects it has been unplugged, it just stops printing and waits to be plugged in again. I don't know how long the battery will last. At one point I measured the current draw of just the M4 Express blinking the LED and decided 420 mAh was enough, but don't have or recall the details.
* [RGB LED](https://www.adafruit.com/product/302). This one is has the diffused lens, which I like. I just picked resistors that gave a good brightness, and R, G, and B are just on or off (no pwmmin').
* [Power Jack](https://www.adafruit.com/product/610) and [Power Supply](https://www.adafruit.com/product/276).
* [Switch](https://www.mouser.com/ProductDetail/979-1901.1102). Any switch will do, but this one fits the 3D printed chassis.
* Resistors for power detection (2 x 100K) and the RGB LED (1.5K, 5.1K, 5.1K).
* Connectors for anything you don't want soldered together. The printer is JST-XH (2.5mm), and I used JST-PH (compatible) elsewhere (2.0mm). For the printer I used bits from one of those JST-XH "kits" off Amazon, and for the rest (JST-PH) I use the connectors-with-tails from Adafruit (e.g. [this](https://www.adafruit.com/product/261) and [this](https://www.adafruit.com/product/3814)).
* There is no PCB; the voltage divider fits on the M4 Express, and the LED resistors are hanging in space.

<img src="/assets/bottom-1.jpg" width="300">
<img src="/assets/bottom-2.jpg" width="300">

## Code

It's all Arduino. I use arduino-cli these days (not the IDE, new or old). You kinda have to know a bit about it already, but here are some bits about how I use it. I use windows, so these are all "cmd.exe" (old-style) batch files. CLI version 0.29 was newest when I did this.
* [Get and install arduino-cli](https://arduino.github.io/arduino-cli/). Drop the executable right there in the "Arduino" directory.
* Edit ac-init.bat according to what you plan to use (it is set for Feather M4 Express as committed). I don't know how to easily find the FQBN for an arbitrary board; running the IDE, enabling verbose output, and plodding through the build output might work.
* Run ac-init.bat. It will get some stuff from Arduino and Adafruit, then create a batch file for the board, namely "ac-feather_m4.bat".
* Run the board batch file. All it does is set an environment variable with the FQBN so the other scripts know how to build/load.
* Run ac-list.bat to look for and list the boards plugged in (so you can see the com port).
* Run ac-build.bat to build and optionally download to the board.
* If needed, run ac-serial.bat to connect to the serial console.

### Running with Only the CPU

No printer, no battery, no power plug. Just a board hanging off USB. This should work with a Feather M4 Express, or with slight modifications to the batch files, an M0 Express, QT Py M0, Xiao, or something of that sort. It just needs 64-bit doubles. It'll compile and run with 32-bit doubles, but the output won't be right.

Edit 2022-11-17_PiMachine.ino:
* WAIT_CONSOLE 1
* PRINT_DIGITS 0
* CHECK_PAPER 0
* CHECK_POWER 0

ac-build 2022-11-17_PiMachine COM*N*

It should build and download.

ac-serial COM*N*

It should connect to the serial port and you should get digit spewage.

### The Software

Stuff shared between sketches is in libraries/PiMachine. That's just how I happen to do it; I can have several test sketches or other variations using the same shared files.
* printer.cpp, printer.h - just enough for this, not for general usage. There might be things in here that don't work in the Adafruit library; I should have fixed and PR'd but didn't. I wouldn't be able to regression test for other firmware versions anyway.
* pidec.cpp, pidec.h - the original source for the nth-digit algorithm, first reformatted (sorry), then converted to run as a subroutine as required for testing and the Pi Machine.
* millis64.cpp, millis64.h - since the idea is to allow it to run for years (ha ha), we need 64 bit milliseconds.
* chrono.cpp, chrono.h - there was a time when I learned and understood std::chrono, and ended up liking it, mostly, iirc. I added this tiny bit of that in response to various subtle problems around pausing and restarting printing (paper change, power unplugged). It's the distinction between time stamps and durations that seems satisfying.
* Sketches - tests for various parts, then the main Pi Machine is in 2022-11-17_PiMachine.
  - Dealing with the printer is split between print_digit() and printer.cpp mentioned previously. Trying to get a digit number, digit, and timestamp on the same line is a little funky, figuring out what that settings mean when text is sideways and such. I think it is the mixing of sideways and not-sideways that causes differences between firmware versions to show up. E.g. one Pi Machine successfully bolds the sideways digit, and one does not.
  
I don't actually understand how the algorithm works, but I've run it to tens of thousands of digits on a Teensy 4.0 and I don't think I've broken it.

### More Hardware

Schematic notes:
* "5V Power Jack" is the 2 amp wall-wart. If this is connected, see POWER NOTE below before plugging in USB or you might be connecting the wal-wart right across your computer's USB port.
* "Switched Battery" is the LiPo, with a switch inline with the + side. The switch is to be able to turn the thing completely off. When running, the switch is normally on so the battery charges. Then when you unplug power, if the switch is on, it sits and waits patiently and does not start all over. If I end up running this thing for a year, I'll probably wish I had put more thought into how the power works.
* 3V3 to the LED comes from the regulator on the Feather. The LED resistors are hanging off the LED, then wired to the CPU board.
* The 100K divider is so the CPU can detect when power is unplugged. It doesn't drop to zero; it drifts down to  3-point-something; enough to detect it. Smaller divider resistors might get it lower, but I did not try. These resistors are mounted to the "Express" area of the CPU board.
* "Printer Serial" is three wires to the JST-XH on the bottom of the printer, and "Printer Power" is two wires going to the other 3-pin JST-XH on the bottom of the printer.

**POWER NOTE:** If the board is powered by a 5V supply you need to **think about** whether you can just plug in USB or not. For the Feather M4 Express, the Pi Machine connects the 5V supply to VUSB, so you should probably not plug in USB at the same time (lest your computer and the wall-wart argue over who can supply more current). I have a USB cable where I cut the +5 wire just for this purpose; you can power with the wall-wart, and plug in the USB cable that just has D+, D-, and GND, and it works nicely.

The chassis was all done in Fusion 360 and printed on a Prusa i3 MK3S+ with PETG.

## Tiny Pi Machine

My grandson likes to push buttons and make things whirr, so I built the Tiny Pi Machine with the "[Nano](https://www.adafruit.com/product/2752)" printer that I ended up not using for the Pi Machine. It does not compute pi; it just prints it as long as the button is down, then starts over and does it again the next time you push the button. Not terribly exciting unless you are 3; then it's easy to go through rolls and rolls of paper.

<img src="/assets/tiny-pi-machine.jpg" width="300">

