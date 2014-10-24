// Simple demo of parsInt Arduino Stream member function.
#include <SdFat.h>

// SD card chip select pin - edit for your SD module.
const uint8_t csPin = 10;

SdFat sd;
SdFile file;
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);
  
  // Initialize the SD.
  if (!sd.begin(csPin)) {
    Serial.println(F("begin error"));
    return;
  }
  // Create and open the file.
  if (!file.open("stream.txt", O_RDWR|O_CREAT|O_TRUNC)) {
    Serial.println(F("open error"));
    return;
  }
  // Write a test number to the file.
  file.println("12345");
  
  // Rewind the file and read the number with parseInt().
  file.rewind(); 
  int i = file.parseInt();
  Serial.print(F("parseInt: "));
  Serial.println(i);
  file.close();
}

void loop() {}
