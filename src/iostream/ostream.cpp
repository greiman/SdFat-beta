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
#include "ostream.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#endif  // __AVR__
#include <string.h>
#ifndef PSTR
#define PSTR(x) x
#endif  // PSTR
//------------------------------------------------------------------------------
void ostream::do_fill(unsigned len) {
  for (; len < width(); len++) {
    putch(fill());
  }
  width(0);
}
//------------------------------------------------------------------------------
void ostream::fill_not_left(unsigned len) {
  if ((flags() & adjustfield) != left) {
    do_fill(len);
  }
}
//------------------------------------------------------------------------------
void ostream::putBool(bool b) {
  if (flags() & boolalpha) {
    if (b) {
      putPgm(PSTR("true"));
    } else {
      putPgm(PSTR("false"));
    }
  } else {
    putChar(b ? '1' : '0');
  }
}
//------------------------------------------------------------------------------
void ostream::putChar(char c) {
  fill_not_left(1);
  putch(c);
  do_fill(1);
}
//------------------------------------------------------------------------------
void ostream::putDouble(double n) {
  uint8_t nd = precision();
  double roundDbl = 0.5;
  char sign;
  char buf[13];  // room for sign, 10 digits, '.', and zero byte
  char *ptr = buf + sizeof(buf) - 1;
  char *str = ptr;
  // terminate string
  *ptr = '\0';

  // get sign and make nonnegative
  if (n < 0.0) {
    sign = '-';
    n = -n;
  } else {
    sign = flags() & showpos ? '+' : '\0';
  }
  // check for larger than uint32_t
  if (n > 4.0E9) {
    putPgm(PSTR("BIG FLT"));
    return;
  }
  // roundDbl up and separate int and fraction parts
  for (uint8_t i = 0; i < nd; ++i) {
    roundDbl *= 0.1;
  }
  n += roundDbl;
  uint32_t intPart = n;
  double fractionPart = n - intPart;

  // format intPart and decimal point
  if (nd || (flags() & showpoint)) {
    *--str = '.';
  }
  str = fmtNum(intPart, str, 10);

  // calculate length for fill
  uint8_t len = sign ? 1 : 0;
  len += nd + ptr - str;

  // extract adjust field
  fmtflags adj = flags() & adjustfield;
  if (adj == internal) {
    if (sign) {
      putch(sign);
    }
    do_fill(len);
  } else {
    // do fill for right
    fill_not_left(len);
    if (sign) {
      *--str = sign;
    }
  }
  putstr(str);
  // output fraction
  while (nd-- > 0) {
    fractionPart *= 10.0;
    int digit = static_cast<int>(fractionPart);
    putch(digit + '0');
    fractionPart -= digit;
  }
  // do fill if not done above
  do_fill(len);
}
//------------------------------------------------------------------------------
void ostream::putNum(int32_t n) {
  bool neg = n < 0 && flagsToBase() == 10;
  putNum(static_cast<uint32_t>(neg ? -n : n), neg);
}
//------------------------------------------------------------------------------
void ostream::putNum(int64_t n) {
  bool neg = n < 0 && flagsToBase() == 10;
  putNum(static_cast<uint64_t>(neg ? -n : n), neg);
}
//------------------------------------------------------------------------------
void ostream::putPgm(const char *str) {
#ifndef __AVR__
  putStr(str);
#else   // __AVR__
  uint8_t c;
  int n;
  for (n = 0; pgm_read_byte(&str[n]); n++) {
  }
  fill_not_left(n);
  for (n = 0; (c = pgm_read_byte(&str[n])); n++) {
    putch(c);
  }
  do_fill(n);
#endif  // __AVR__
}
//------------------------------------------------------------------------------
void ostream::putStr(const char *str) {
  unsigned n = strlen(str);
  fill_not_left(n);
  putstr(str);
  do_fill(n);
}
