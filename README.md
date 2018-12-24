### Warning: This beta is for testing with Particle IoT mesh devices.

Rename this folder SdFat and place it in the standard place for libraries.

I tested with a particle CLI project with a SdFat in a lib

```
 \Users\bill\Documents\Particle\projects\SdFatMod>ls *
README.md  argon_firmware_1545324931631.bin  project.properties

lib:
SdFat

src:
SdFatMod.ino
```
Changess Version 1.0.10:

Added custom Particle driver to use DMA and allow use of SPI1.

To use SPI1 declare SdFat/SdFatEX class like this:

```
SdFat sd1(&SPI1);
// or
SdFatEX sd1(&SPI1);
```
Changed STM32 use of SPI ports.  See STM32Test example.

To use second SPI port:
```
// Use second SPI port
SPIClass SPI_2(2);
SdFat sd2(&SPI_2);
```

Changes Version 1.0.9:

This version of SdFat has been modified to use standard POSIX/Linux 
definitions of open flags from fcntl.h.

Open flags are access modes, O_RDONLY, O_RDWR, O_WRONLY, and modifiers
O_APPEND, O_CREAT, O_EXCL, O_SYNC, O_TRUNC.

The mods required changing the type for open flags from uint8_t to int so
bugs are likely if any uint8_t local variables were missed.


