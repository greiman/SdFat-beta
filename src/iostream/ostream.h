/**
 * Copyright (c) 2011-2025 Bill Greiman
 * This file is part of the SdFat library for SD memory cards.
 *
 * MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#pragma once
/**
 * \file
 * \brief \ref ostream class
 */
#include "ios.h"
//==============================================================================
/**
 * \class ostream
 * \brief Output Stream
 */
class ostream : public virtual ios {
 public:
  ostream() {}

  /** call manipulator
   * \param[in] pf function to call
   * \return the stream
   */
  ostream &operator<<(ostream &(*pf)(ostream &str)) { return pf(*this); }
  /** call manipulator
   * \param[in] pf function to call
   * \return the stream
   */
  ostream &operator<<(ios_base &(*pf)(ios_base &str)) {
    pf(*this);
    return *this;
  }
  /** Output bool
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(bool arg) {
    putBool(arg);
    return *this;
  }
  /** Output string
   * \param[in] arg string to output
   * \return the stream
   */
  ostream &operator<<(const char *arg) {
    putStr(arg);
    return *this;
  }
  /** Output string
   * \param[in] arg string to output
   * \return the stream
   */
  ostream &operator<<(const signed char *arg) {
    putStr(reinterpret_cast<const char *>(arg));
    return *this;
  }
  /** Output string
   * \param[in] arg string to output
   * \return the stream
   */
  ostream &operator<<(const unsigned char *arg) {
    putStr(reinterpret_cast<const char *>(arg));
    return *this;
  }
#if ENABLE_ARDUINO_STRING
  /** Output string
   * \param[in] arg string to output
   * \return the stream
   */
  ostream &operator<<(const String &arg) {
    putStr(arg.c_str());
    return *this;
  }
#endif  // ENABLE_ARDUINO_STRING
  /** Output character
   * \param[in] arg character to output
   * \return the stream
   */
  ostream &operator<<(char arg) {
    putChar(arg);
    return *this;
  }
  /** Output character
   * \param[in] arg character to output
   * \return the stream
   */
  ostream &operator<<(signed char arg) {
    putChar(static_cast<char>(arg));
    return *this;
  }
  /** Output character
   * \param[in] arg character to output
   * \return the stream
   */
  ostream &operator<<(unsigned char arg) {
    putChar(static_cast<char>(arg));
    return *this;
  }
  /** Output double
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(double arg) {
    putDouble(arg);
    return *this;
  }
  /** Output float
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(float arg) {
    putDouble(arg);
    return *this;
  }
  /** Output signed short
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(short arg) {  // NOLINT
    putNum(static_cast<int32_t>(arg));
    return *this;
  }
  /** Output unsigned short
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(unsigned short arg) {  // NOLINT
    putNum(static_cast<uint32_t>(arg));
    return *this;
  }
  /** Output signed int
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(int arg) {
    putNum(static_cast<int32_t>(arg));
    return *this;
  }
  /** Output unsigned int
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(unsigned int arg) {
    putNum(static_cast<uint32_t>(arg));
    return *this;
  }
  /** Output signed long
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(long arg) {  // NOLINT
    putNum(static_cast<int32_t>(arg));
    return *this;
  }
  /** Output unsigned long
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(unsigned long arg) {  // NOLINT
    putNum(static_cast<uint32_t>(arg));
    return *this;
  }
  /** Output signed long long
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(long long arg) {  // NOLINT
    putNum(static_cast<int64_t>(arg));
    return *this;
  }
  /** Output unsigned long long
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(unsigned long long arg) {  // NOLINT
    putNum(static_cast<uint64_t>(arg));
    return *this;
  }
  /** Output pointer
   * \param[in] arg value to output
   * \return the stream
   */
  ostream &operator<<(const void *arg) {
    putNum(reinterpret_cast<uint32_t>(arg));
    return *this;
  }
  /** Output a string from flash using the Arduino F() macro.
   * \param[in] arg pointing to flash string
   * \return the stream
   */
  ostream &operator<<(const __FlashStringHelper *arg) {
    putPgm(reinterpret_cast<const char *>(arg));
    return *this;
  }
  /**
   * Puts a character in a stream.
   *
   * The unformatted output function inserts the element \a ch.
   * It returns *this.
   *
   * \param[in] ch The character
   * \return A reference to the ostream object.
   */
  ostream &put(char ch) {
    putch(ch);
    return *this;
  }
  //  ostream& write(char *str, streamsize count);
  /**
   * Flushes the buffer associated with this stream. The flush function
   * calls the sync function of the associated file.
   * \return A reference to the ostream object.
   */
  ostream &flush() {
    if (!sync()) {
      setstate(badbit);
    }
    return *this;
  }
  /**
   * \return the stream position
   */
  pos_type tellp() { return tellpos(); }
  /**
   * Set the stream position
   * \param[in] pos The absolute position in which to move the write pointer.
   * \return Is always *this.  Failure is indicated by the state of *this.
   */
  ostream &seekp(pos_type pos) {
    if (!seekpos(pos)) {
      setstate(failbit);
    }
    return *this;
  }
  /**
   * Set the stream position.
   *
   * \param[in] off An offset to move the write pointer relative to way.
   * \a off is a signed 32-bit int so the offset is limited to +- 2GB.
   * \param[in] way One of ios::beg, ios::cur, or ios::end.
   * \return Is always *this.  Failure is indicated by the state of *this.
   */
  ostream &seekp(off_type off, seekdir way) {
    if (!seekoff(off, way)) {
      setstate(failbit);
    }
    return *this;
  }

 protected:
  /// @cond SHOW_PROTECTED
  /** Put character with binary/text conversion
   * \param[in] ch character to write
   */
  virtual void putch(char ch) = 0;
  virtual void putstr(const char *str) = 0;
  virtual bool seekoff(off_type pos, seekdir way) = 0;
  virtual bool seekpos(pos_type pos) = 0;
  virtual bool sync() = 0;
  virtual pos_type tellpos() = 0;
  /// @endcond
 private:
  void do_fill(unsigned len);
  void fill_not_left(unsigned len);
  void putBool(bool b);
  void putChar(char c);
  void putDouble(double n);
  void putNum(int32_t n);
  void putNum(int64_t n);
  void putNum(uint32_t n) { putNum(n, false); }
  void putNum(uint64_t n) { putNum(n, false); }
  void putPgm(const char *str);
  void putStr(const char *str);

  template <typename T>
  char *fmtNum(T n, char *ptr, uint8_t base) {
    char a = (flags() & uppercase) ? 'A' - 10 : 'a' - 10;
    do {
      T m = n;
      n /= base;
      char c = m - base * n;
      *--ptr = c < 10 ? c + '0' : c + a;
    } while (n);
    return ptr;
  }

  template <typename T>
  void putNum(T n, bool neg) {
    char buf[(8 * sizeof(T) + 2) / 3 + 2];
    char *ptr = buf + sizeof(buf) - 1;
    char *num;
    char *str;
    uint8_t base = flagsToBase();
    *ptr = '\0';
    str = num = fmtNum(n, ptr, base);
    if (base == 10) {
      if (neg) {
        *--str = '-';
      } else if (flags() & showpos) {
        *--str = '+';
      }
    } else if (flags() & showbase) {
      if (flags() & hex) {
        *--str = flags() & uppercase ? 'X' : 'x';
      }
      *--str = '0';
    }
    uint8_t len = ptr - str;
    fmtflags adj = flags() & adjustfield;
    if (adj == internal) {
      while (str < num) {
        putch(*str++);
      }
      do_fill(len);
    } else {
      // do fill for right
      fill_not_left(len);
    }
    putstr(str);
    do_fill(len);
  }
};
