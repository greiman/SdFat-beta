/* Arduino SdFat Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the Arduino SdFat Library
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdFat Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
/**
 * \file
 * \brief SdFile class
 */
#ifndef SdFile_h
#define SdFile_h
#include <limits.h>
#include <SdBaseFile.h>
//------------------------------------------------------------------------------
/**
 * \class SdFile
 * \brief SdBaseFile with Arduino Stream.
 */
class SdFile : public SdBaseFile, public Stream {
 public:
  SdFile() {}
  SdFile(const char* name, uint8_t oflag);
#if DESTRUCTOR_CLOSES_FILE
  ~SdFile() {}
#endif  // DESTRUCTOR_CLOSES_FILE
  /** \return number of bytes available from the current position to EOF
   *   or INT_MAX if more than INT_MAX bytes are available.
   */
  int available() {
    uint32_t n = SdBaseFile::available();
    return n > INT_MAX ? INT_MAX : n;
  }
  /** Ensure that any bytes written to the file are saved to the SD card. */
  void flush() {SdBaseFile::sync();} 
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */  
  int peek() {return SdBaseFile::peek();}
  /** Read the next byte from a file.
   *
   * \return For success return the next byte in the file as an int.
   * If an error occurs or end of file is reached return -1.
   */  
  int read() {return SdBaseFile::read();}
  /** \return value of writeError */
  bool getWriteError() {return SdBaseFile::getWriteError();}
  /** Set writeError to zero */
  void clearWriteError() {SdBaseFile::clearWriteError();}
  size_t write(uint8_t b);
  int write(const char* str);
  int write(const void* buf, size_t nbyte);
  size_t write(const uint8_t *buf, size_t size) {
    return SdBaseFile::write(buf, size);}
  void write_P(PGM_P str);
  void writeln_P(PGM_P str);
};
#endif  // SdFile_h
