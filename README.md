### Warning: This beta is for testing with Particle IoT mesh devices.

This version of SdFat has been modified to use standard POSIX/Linux 
definitions of open flags from fcntl.h.

Open flags are access modes, O_RDONLY, O_RDWR, O_WRONLY, and modifiers
O_APPEND, O_CREAT, O_EXCL, O_SYNC, O_TRUNC.

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

The mods required changing the type for open flags from uint8_t to int so
bugs are likely if any uint8_t local variables were missed.


