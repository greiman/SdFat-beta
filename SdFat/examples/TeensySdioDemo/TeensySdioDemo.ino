// Simple performance test for Teensy 3.5/3.6 SDHC.
// Demonstrates yield() efficiency.

#include "SdFat.h"

// 32 KiB buffer.
const size_t BUF_DIM = 32768;

// 8 MiB file.
const uint32_t FILE_SIZE = 256UL*BUF_DIM;

SdFatSdio sd;

File file;

uint8_t buf[BUF_DIM];

// buffer as uint32_t
uint32_t* buf32 = (uint32_t*)buf;

// Total usec in read/write calls.
uint32_t totalMicros = 0;
// Time in yield() function.
uint32_t yieldMicros = 0;
// Number of yield calls.
uint32_t yieldCalls = 0;
// Max busy time for single yield call.
uint32_t yieldMaxUsec = 0; 
//-----------------------------------------------------------------------------
// Replace "weak" system yield() function.
void yield() {
  // Only count cardBusy time.
  if (!sd.card()->dmaBusy()) {
    return;
 }
  uint32_t m = micros();
  yieldCalls++;
  while (sd.card()->dmaBusy()) {
    // Do something here.
  }
  m = micros() - m;
  if (m > yieldMaxUsec) {
    yieldMaxUsec = m;
  }
  yieldMicros += m;
}
//-----------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {
  }
  Serial.println("Type any character to begin");
  while (!Serial.available()) {
  }
  if (!sd.begin()) {
    sd.initErrorHalt();
  }
  if (!file.open("TeensyDemo.bin", O_RDWR | O_CREAT)) {
    sd.errorHalt("open failed");
  }
  Serial.println("\nsize,write,read");
  Serial.println("bytes,KB/sec,KB/sec");
  for (size_t nb = 512; nb <= BUF_DIM; nb *= 2) {
    file.truncate(0);
    uint32_t nRdWr = FILE_SIZE/nb;
    Serial.print(nb);
    Serial.print(',');
    uint32_t t = micros();
    for (uint32_t n = 0; n < nRdWr; n++) {
      // Set start and end of buffer.
      buf32[0] = n;
      buf32[nb/4 - 1] = n;
      if (nb != file.write(buf, nb)) {
        sd.errorHalt("write failed");
      }
    }
    t = micros() - t;
    totalMicros += t;
    Serial.print(1000.0*FILE_SIZE/t);
    Serial.print(',');
    file.rewind();
    t = micros();
    
    for (uint32_t n = 0; n < nRdWr; n++) {
      if ((int)nb != file.read(buf, nb)) {
        sd.errorHalt("read failed");
      }
      // crude check of data.     
      if (buf32[0] != n || buf32[nb/4 - 1] != n) {
        sd.errorHalt("data check");
      }
    }
    t = micros() - t;
    totalMicros += t;   
    Serial.println(1000.0*FILE_SIZE/t);    
  }
  file.close();
  Serial.print("\ntotalMicros  ");
  Serial.println(totalMicros);
  Serial.print("yieldMicros  ");
  Serial.println(yieldMicros);
  Serial.print("yieldCalls   ");
  Serial.println(yieldCalls);
  Serial.print("yieldMaxUsec ");
  Serial.println(yieldMaxUsec);
  Serial.print("kHzSdClk     ");
  Serial.println(sd.card()->kHzSdClk());
  Serial.println("Done");
}

void loop() {
}
