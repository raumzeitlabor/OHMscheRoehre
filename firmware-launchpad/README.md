This is the firmware for the TI Stellaris Launchpad for the OHMsche Roehre. It is based on a regular TI example script, thus the comments inside the file seem to be odd.

The firmware parses the key states from the front panel and sends them via RS232 to the OHM2013 loc controller.

In order to compile this firmware, you need:

- The GNU Tools for ARM Embedded Processors from https://launchpad.net/gcc-arm-embedded
- The lm4flash tool from https://github.com/utzig/lm4tools
- The StellarisWare sources from http://www.ti.com/tool/sw-ek-lm4f120xl (Part Number is SW-EK-LM4F120XL)

Copy this directory into the StellarisWare boards/ek-lm4f120xl directory, run make, then flash the file gcc/firmware-launchpad.bin file.
