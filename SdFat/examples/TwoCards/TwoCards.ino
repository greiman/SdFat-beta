/*
 * Example use of two SD cards.
 */
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>

SdFat sd1;
const uint8_t SD1_CS = 10;  // chip select for sd1

SdFat sd2;
const uint8_t SD2_CS = 9;   // chip select for sd2

const uint8_t BUF_DIM = 100;
uint8_t buf[BUF_DIM];

const uint32_t FILE_SIZE = 1000000;
const uint16_t NWRITE = FILE_SIZE/BUF_DIM;
//------------------------------------------------------------------------------
// print error msg, any SD error codes, and halt.
// store messages in flash
#define errorExit(msg) errorHalt(F(msg))
#define initError(msg) initErrorHalt(F(msg))
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial) {}  // wait for Leonardo
  Serial.print(F("FreeRam: "));

  Serial.println(FreeRam());
  
  // fill buffer with known data
  for (int i = 0; i < sizeof(buf); i++) buf[i] = i;
  
  Serial.println(F("type any character to start"));
  while (Serial.read() <= 0) {}
  delay(400);  // catch Due reset problem

  // disable sd2 while initializing sd1
  pinMode(SD2_CS, OUTPUT);
  digitalWrite(SD2_CS, HIGH);
  
  // initialize the first card
  if (!sd1.begin(SD1_CS)) {
    sd1.initError("sd1:");
  }
  // create DIR1 on sd1 if it does not exist
  if (!sd1.exists("/DIR1")) {
    if (!sd1.mkdir("/DIR1")) sd1.errorExit("sd1.mkdir");
  }
  // initialize the second card
  if (!sd2.begin(SD2_CS)) {
    sd2.initError("sd2:");
  }
 // create DIR2 on sd2 if it does not exist
  if (!sd2.exists("/DIR2")) {
    if (!sd2.mkdir("/DIR2")) sd2.errorExit("sd2.mkdir");
  }
  // list root directory on both cards
  Serial.println(F("------sd1 root-------"));
  sd1.ls();
  Serial.println(F("------sd2 root-------"));
  sd2.ls();

  // make /DIR1 the default directory for sd1
  if (!sd1.chdir("/DIR1")) sd1.errorExit("sd1.chdir");
  
  // make /DIR2 the default directory for sd2
  if (!sd2.chdir("/DIR2")) sd2.errorExit("sd2.chdir");
  
  // list current directory on both cards
  Serial.println(F("------sd1 DIR1-------"));
  sd1.ls();
  Serial.println(F("------sd2 DIR2-------"));
  sd2.ls();
  Serial.println(F("---------------------"));
  
  // remove RENAME.BIN from /DIR2 directory of sd2
  if (sd2.exists("RENAME.BIN")) {
    if (!sd2.remove("RENAME.BIN")) {
      sd2.errorExit("remove RENAME.BIN");
    }
  }
  // set the current working directory for open() to sd1
  sd1.chvol();
  
  // create or open /DIR1/TEST.BIN and truncate it to zero length
  SdFile file1;
  if (!file1.open("TEST.BIN", O_RDWR | O_CREAT | O_TRUNC)) {
    sd1.errorExit("file1");
  }
  Serial.println(F("Writing TEST.BIN to sd1"));
  
  // write data to /DIR1/TEST.BIN on sd1
  for (int i = 0; i < NWRITE; i++) {
    if (file1.write(buf, sizeof(buf)) != sizeof(buf)) {
      sd1.errorExit("sd1.write");
    }
  }
  // set the current working directory for open() to sd2
  sd2.chvol();
  
  // create or open /DIR2/COPY.BIN and truncate it to zero length
  SdFile file2;
  if (!file2.open("COPY.BIN", O_WRITE | O_CREAT | O_TRUNC)) {
    sd2.errorExit("file2");
  }
  Serial.println(F("Copying TEST.BIN to COPY.BIN"));
  
  // copy file1 to file2
  file1.rewind();
  uint32_t t = millis();

  while (1) {
    int n = file1.read(buf, sizeof(buf));
    if (n < 0) sd1.errorExit("read1");
    if (n == 0) break;
    if (file2.write(buf, n) != n) sd2.errorExit("write2");
  }
  t = millis() - t;
  Serial.print(F("File size: "));
  Serial.println(file2.fileSize());
  Serial.print(F("Copy time: "));
  Serial.print(t);
  Serial.println(F(" millis"));
  
  // close TEST.BIN
  file1.close();
  
  // rename the copy
  file2.close();
  if (!sd2.rename("COPY.BIN", "RENAME.BIN")) {
    sd2.errorExit("sd2.rename");
  }
  Serial.println(F("Done"));
}
//------------------------------------------------------------------------------
void loop() {}
