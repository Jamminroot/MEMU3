# MEMU3 - (M)ouse (EMU)lator, v.3

## About

MEMU3 - Software to look for specific color on screen, and emulate mouse actions through mouse driver. Might be used as an aim-assist, for example - in Overwatch.

Runs on Interception mouse filter driver and uses direct-x API for overlay.

Feel free to use in any way you like - but be aware that it's just a POC.

## Installation

In order for software itself to work you'd need to install interception mouse driver. Refer to [Oblitum's Interception](https://github.com/oblitum/Interception) repository for details.

## Controls

F2 - Change mode

Num-8 - Use different color settings (For version 1.0 I've included pre-calculated settings for red (enemy) and neon (allies) colors) (it's those obnoxious files 2mb files)

ALT - Toggle On/Off

Num-Del - Change Debug UI mode

Num-0 - Change UI mode

Num-Plus \ Num-Minus - Increase\decrease strength (or mode-specific value)

Num-9 \ Num-3 - Increase\decrease sensitivity (affects flickshots)

---

## Color configs

Target colors are described in `.colorset` files, those are CSV (comma-separated values) : B,G,R,T where R stands for RED, G stands for GREEN, B stands for BLUE and T stands for TOLERANCE.

### Sample contents

```csv
241,235,94,16
241,232,85,16
250,243,109,16
219,219,63,16
228,221,56,16
```

For example, `241,235,94,16` means that software is going to search for color rgb(94,235,241) (neon-ish color) and every color which are 16 points "around" it anchor color.

### Threshold math

Target color: `100,100,100,10`, scanning pixel (103,100,100):

sqrt((103-100)^2+(100-100)^2+(100-100)^2) = sqrt(9) = 3, which is lower than tolerance 10: color would pass the threshold.

Target color: `100,100,100,3`, scanning pixel (104,100,100):

sqrt((104-100)^2+(100-100)^2+(100-100)^2) = sqrt(16) = 4, which is higher than tolerance 3: color would not pass the threshold.

---

## Build

Build it with Cmake, should be straight-forward. 

Make sure to verify CMakeLists.txt paths (DirectX SDK, interception headers\binaries), too.

## Contribute

Feel free to fork\open PR if there's anything to improve. Cheers o/
