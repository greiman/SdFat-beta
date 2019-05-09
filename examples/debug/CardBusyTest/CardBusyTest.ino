#include "SdFat.h"

#ifdef __AVR__
const uint32_t FILE_SIZE_MiB = 10UL;
#else  // __AVR__
const uint32_t FILE_SIZE_MiB = 100UL;
#endif

bool waitBusy = true;

#define SD_CONFIG SdSpiConfig(SS, DEDICATED_SPI)
// Config for Teensy 3.5/3.6 buit-in SD.
//#define SD_CONFIG SdSpiConfig(SDCARD_SS_PIN, DEDICATED_SPI)
//#define SD_CONFIG SdioConfig(FIFO_SDIO)

//------------------------------------------------------------------------------
const uint64_t FILE_SIZE = (uint64_t)FILE_SIZE_MiB  << 20;

SdExFat sd;
ExFile file;

uint8_t buf[512];

#define error(s) sd.errorHalt(&Serial, F(s))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Wait for USB Serial
  while (!Serial) {
    SysCall::yield();
  }
  delay(1000);

  Serial.println(F("Type any character to start\n"));
  while (!Serial.available()) {
    SysCall::yield();
  }
  // Initialize the SD card.
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt();
  }
  if (!file.open("SdBusyTest.bin", O_RDWR | O_CREAT |O_TRUNC)) {
    error("file open failed");
  }
  if (!file.preAllocate(FILE_SIZE)) {
    error("preallocate failed");
  }
  Serial.print(F("Starting write of "));
  Serial.print(FILE_SIZE_MiB);
  Serial.println(F(" MiB."));
  uint32_t maxMicros = 0;
  uint32_t minMicros = 99999999;
  uint32_t ms = millis();

  // Write a dummy sector to start a multi-sector write.
  if(file.write(buf, sizeof(buf)) != sizeof(buf)) {
    error("write failed for first sector");
  }

  while (file.position() < FILE_SIZE) {
    if (waitBusy) {
      while (sd.card()->isBusy()) {}
    }
    uint32_t m = micros();
    if (file.write(buf, sizeof(buf)) != sizeof(buf)) {
      error("write failed");
    }
    m = micros() - m;
    if (m < minMicros) {
      minMicros = m;
    }
    if (m > maxMicros) {
      maxMicros = m;
    }
  }
  ms = millis() - ms;
  Serial.print(F("minMicros: "));
  Serial.println(minMicros);
  Serial.print(F("maxMicros: "));
  Serial.println(maxMicros);
  Serial.print(1e-3*ms);
  Serial.println(F(" Seconds"));
  Serial.print(1.0*FILE_SIZE/ms);
  Serial.println(F(" KB/sec"));
}
void loop() {}
