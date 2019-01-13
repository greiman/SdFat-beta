### Warning: This beta is no longer up to date with the release SdFat.

### Please use the current version of SdFat.

Changes Version 1.0.10:

Initial test version for Particle Gen3 mesh.

Changes Version 1.0.9:

This version of SdFat has been modified to use standard POSIX/Linux 
definitions of open flags from fcntl.h.

Open flags are access modes, O_RDONLY, O_RDWR, O_WRONLY, and modifiers
O_APPEND, O_CREAT, O_EXCL, O_SYNC, O_TRUNC.

The mods required changing the type for open flags from uint8_t to int so
bugs are likely if any uint8_t local variables were missed.


