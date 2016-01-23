
// Function to read a CSV text file one field at a time.
//
#include <SPI.h>
#include <SdFat.h>
#define CS_PIN 10

SdFat SD;
File file;

/*
 * Read a file one field at a time.
 *
 * file - File to read.
 *
 * str - Character array for the field.
 *
 * size - Size of str array.
 *
 * delim - String containing field delimiters.
 *
 * return - length of field including terminating delimiter.
 *
 * Note, the last character of str will not be a delimiter if
 * a read error occurs, the field is too long, or the file
 * does not end with a delimiter.  Consider this an error
 * if not at end-of-file.
 *
 */
size_t readField(File* file, char* str, size_t size, char* delim) {
  char ch;
  size_t n = 0;
  while ((n + 1) < size && file->read(&ch, 1) == 1) {
    // Delete CR.
    if (ch == '\r') {
      continue;
    }
    str[n++] = ch;
    if (strchr(delim, ch)) {
        break;
    }
  }
  str[n] = '\0';
  return n;
}
//------------------------------------------------------------------------------
#define errorHalt(msg) {Serial.println(F(msg)); while(1);}
//------------------------------------------------------------------------------
void setup() {
  Serial.begin(9600);

  // Initialize the SD.
  if (!SD.begin(CS_PIN)) errorHalt("begin failed");

  // Create or open the file.
  file = SD.open("READTEST.TXT", FILE_WRITE);
  if (!file) errorHalt("open failed");

  // Rewind file so test data is not appended.
  file.seek(0);

  // Write test data.
  file.print(F(
    "field_1_1,field_1_2,field_1_3\r\n"
    "field_2_1,field_2_2,field_2_3\r\n"
    "field_3_1,field_3_2\r\n"           // missing a field
    "field_4_1,field_4_2,field_4_3\r\n"
    "field_5_1,field_5_2,field_5_3"     // no delimiter
    ));

  // Rewind the file for read.
  file.seek(0);

  size_t n;      // Length of returned field with delimiter.
  char str[20];  // Must hold longest field with delimiter and zero byte.
  
  // Read the file and print fields.
  while (true) {
    n = readField(&file, str, sizeof(str), ",\n");

    // done if Error or at EOF.
    if (n == 0) break;

    // Print the type of delimiter.
    if (str[n-1] == ',' || str[n-1] == '\n') {
      Serial.print(str[n-1] == ',' ? F("comma: ") : F("endl:  "));
      
      // Remove the delimiter.
      str[n-1] = 0;
    } else {
      // At eof, too long, or read error.  Too long is error.
      Serial.print(file.available() ? F("error: ") : F("eof:   "));
    }
    // Print the field.
    Serial.println(str);
  }
  file.close();
}
//------------------------------------------------------------------------------
void loop() {
}


