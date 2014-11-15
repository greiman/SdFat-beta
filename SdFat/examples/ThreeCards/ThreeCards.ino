/*
 * Example use of three SD cards.
 */
#include <SPI.h>
#include <SdFat.h>
#include <SdFatUtil.h>
#if USE_MULTIPLE_SPI_TYPES  // Must be nonzero in SdFat/SdFatConfig.h

// SD1 is a microSD on hardware SPI pins 50-52
// Using my fast custom SPI
SdFat sd1;
const uint8_t SD1_CS = 53;
  
// SD2 is a Catalex shield on hardware SPI pins 50-52
// Using the standard Arduino SPI library
SdFatLibSpi sd2;
const uint8_t SD2_CS = 4;

// SD3 is a Adafruit data logging shield on pins 10-13
// Using Software SPI
SdFatSoftSpi<12, 11, 13> sd3;
const uint8_t SD3_CS = 10;

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
void list() {
 // list current directory on both cards
  Serial.println(F("------sd1-------"));
  sd1.ls("/", LS_SIZE|LS_R);
  Serial.println(F("------sd2-------"));
  sd2.ls("/", LS_SIZE|LS_R);
  Serial.println(F("------sd3-------"));
  sd3.ls("/", LS_SIZE|LS_R);  
  Serial.println(F("---------------------"));    
}
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

  // disable sd2 while initializing sd1
  pinMode(SD2_CS, OUTPUT);
  digitalWrite(SD2_CS, HIGH);
  
  // initialize the first card
  if (!sd1.begin(SD1_CS)) sd1.initError("sd1:");
  
  // initialize the second card
  if (!sd2.begin(SD2_CS)) sd2.initError("sd2:");
 
  // initialize the third card
  if (!sd3.begin(SD3_CS)) sd3.initError("sd3:");
  
  Serial.println(F("Cards OK - creating directories"));
  
  // create DIR1 on sd1 if it does not exist
  if (!sd1.exists("/DIR1")) {
    if (!sd1.mkdir("/DIR1")) sd1.errorExit("sd1.mkdir");
  }  
  // make /DIR1 the default directory for sd1
  if (!sd1.chdir("/DIR1")) sd1.errorExit("sd1.chdir");
  
  // create DIR2 on sd2 if it does not exist
  if (!sd2.exists("/DIR2")) {
    if (!sd2.mkdir("/DIR2")) sd2.errorExit("sd2.mkdir");
  }
  // make /DIR2 the default directory for sd2
  if (!sd2.chdir("/DIR2")) sd2.errorExit("sd2.chdir");
  
  // create DIR3 on sd3 if it does not exist
  if (!sd3.exists("/DIR3")) {
    if (!sd3.mkdir("/DIR3")) sd2.errorExit("sd3.mkdir");
  }
  // make /DIR3 the default directory for sd3
  if (!sd3.chdir("/DIR3")) sd3.errorExit("sd3.chdir");
 
  Serial.println(F("Directories created - removing old files"));
  
  if (sd1.exists("TEST1.BIN")) {
    if (!sd1.remove("TEST1.BIN")) sd1.errorExit("sd1.remove");
  }  
  if (sd2.exists("TEST2.BIN")) {
    if (!sd2.remove("TEST2.BIN")) sd2.errorExit("sd2.remove");
  } 
  if (sd3.exists("TEST3.BIN")) {
    if (!sd3.remove("TEST3.BIN")) sd2.errorExit("sd3.remove");
  } 
  Serial.println("Initial SD directories");
  list();

  // create or open /DIR1/TEST1.BIN and truncate it to zero length
  SdFile file1;
  if (!file1.open(&sd1, "TEST1.BIN", O_RDWR | O_CREAT | O_TRUNC)) {
    sd1.errorExit("file1");
  }
  Serial.println(F("Writing SD1:/DIR1/TEST1.BIN"));
  
  // write data to /DIR1/TEST.BIN on sd1
  for (int i = 0; i < NWRITE; i++) {
    if (file1.write(buf, sizeof(buf)) != sizeof(buf)) {
      sd1.errorExit("sd1.write");
    }
  }
  file1.sync();
  list();
  
  // create or open /DIR2/TEST2.BIN and truncate it to zero length
  SdFile file2;
  if (!file2.open(&sd2, "TEST2.BIN", O_RDWR | O_CREAT | O_TRUNC)) {
    sd2.errorExit("file2");
  }
  Serial.println(F("Copying SD1:/DIR1/TEST1.BIN to SD2::/DIR2/TEST2.BIN"));
  
  // copy file1 to file2
  file1.rewind();
  
  uint32_t t = millis();

  while (1) {
    int n = file1.read(buf, sizeof(buf));
    if (n < 0) sd1.errorExit("read1");
    if (n == 0) break;
    if (file2.write(buf, n) != n) sd2.errorExit("write3");
  }
  t = millis() - t;
  file2.sync();
  Serial.print(F("File size: "));
  Serial.println(file2.fileSize());
  Serial.print(F("Copy time: "));
  Serial.print(t);
  Serial.println(F(" millis"));
  list();
  
  // create or open /DIR3/TEST3.BIN and truncate it to zero length
  SdFile file3;
  if (!file3.open(&sd3, "TEST3.BIN", O_RDWR | O_CREAT | O_TRUNC)) {
    sd3.errorExit("file3");
  }
  file2.rewind();
  Serial.println(F("Copying SD2:/DIR2/TEST2.BIN to SD3:/DIR3/TEST3.BIN"));  
  while (1) {
    int n = file2.read(buf, sizeof(buf));
    if (n == 0) break;   
    if (n != sizeof(buf)) sd2.errorExit("read2");
    if (file3.write(buf, n) != n) sd3.errorExit("write2");
  }
  file3.sync();
  list();
  
  // Verify content of file3
  file3.rewind();
  Serial.println(F("Verifying content of TEST3.BIN"));
  for (int i = 0; i < NWRITE; i++) {
    if (file3.read(buf, sizeof(buf)) != sizeof(buf)) {
      sd3.errorExit("sd3.read");
    }
    for (int j = 0; j < sizeof(buf); j++) {
      if (j != buf[j]) sd3.errorExit("Verify error");
    }
  }
  Serial.println(F("Done - Verify OK"));
  file1.close();
  file2.close();
  file3.close();
}
//------------------------------------------------------------------------------
void loop() {}
#else  // USE_MULTIPLE_SPI_TYPES
#error USE_MULTIPLE_SPI_TYPES must be set nonzero in SdFat/SdFatConfig.h
#endif  //USE_MULTIPLE_SPI_TYPES
