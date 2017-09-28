### Warning: This beta version is now out of date. Use standard version of SdFat.

Teensy 3.5/3.6 SDIO support has been added.  Try the TeensySdioDemo example.
Many other example will work with Teensy SDIO if you use the SdFatSdio classes
and call begin with no parameters.

```
 SdFatSdio sd;
 
 ....
 
  if (!sd.begin()) {
    // Handle failure.
  }
 
```

Recent versions of the Arduino IDE have bugs that may cause SdFat-beta to crash.

https://forum.arduino.cc/index.php?topic=419264.0

SdFat-beta was tested using Arduino AVR boards 1.6.11. 
If you are using IDE 1.6.11 you must also install AVR boards 1.6.11, not
1.6.12 or 1.6.13.

If you are using Arduino IDE 1.6.11, do Tools > Board > Boards Manager > Arduino AVR Boards > 1.6.11 > Install > Close.

Key changes:

The SPI divisor has been replaced by SPISettings.

There are two new classes, SdFatEX and SdFatSoftSpiEX.

Please read changes.txt and the html documentation for more information.

Please report problems as issues.

The Arduino SdFat library provides read/write access to FAT16/FAT32
file systems on SD/SDHC flash cards.

SdFat requires Arduino 1.6x or greater.

To use SdFat, unzip the download file and place the SdFat folder
into the libraries sub-folder in your main sketch folder.

For more information see the Manual installation section of this guide:

http://arduino.cc/en/Guide/Libraries 

A number of configuration options can be set by editing SdFatConfig.h
define macros.  See the html documentation for details

Read changes.txt if you have used previous releases of this library.

Please read the html documentation for this library.  Start with
html/index.html and read the Main Page.  Next go to the Classes tab and
read the documentation for the classes SdFat, SdFatEX, SdBaseFile,
SdFile, File, StdioStream, ifstream, ofstream, and others.
 
Please continue by reading the html documentation.

Updated 5 Sep 2016
