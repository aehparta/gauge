# gauge
Gauge is a software collection that originally started as a digital analysator library
and separate multimeter application for android. Everything has been since merged into
the same repository. Multimeter application itself has evolved into something able
to control even the lights in my apartment.

# [devlib](devlib)
Library for managing devices.

# [modules](modules)
Device driver modules for devlib.

## [dev-test](modules/dev-test)
Input test device driver:
* 1 kHz 8 channel digital counter
* 1 kHz sine wave
* 1 kHz sawtooth 

## [dev-ftd2xx](modules/dev-ftd2xx)
FTDI driver.
Supports:
* [FT232H](http://www.ftdichip.com/Products/ICs/FT232H.htm)

## [skeleton](modules/skeleton)
Empty module skeleton.

# [gui](gui)
Graphical user interface for devlib. OpenGL 3.3 and Qt 5.7 required.

First two signals are real 1 kHz data captured using FT232H based digital analysator with 1 MS/s:
![First real data capture](http://up.zeraw.biz/dl/GW05s8/first-real-data.png)

# [multimeter](multimeter)
Simple multimeter application that supports bluetooth devices. Work on android and linux.
Windows support might be available in the future when bluetooth starts working with Qt in windows.
![Example](http://up.zeraw.biz/dl/naCA6A/screenshot4.png)
