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
#include "StdioStream.h"
#ifdef __AVR__
#include <avr/pgmspace.h>
#endif  // __AVR__
#include "../common/FmtNumber.h"
//------------------------------------------------------------------------------
int StdioStream::fclose() {
  int rtn = 0;
  if (!m_status) {
    return EOF;
  }
  if (m_status & S_SWR) {
    if (!flushBuf()) {
      rtn = EOF;
    }
  }
  if (!StreamBaseFile::close()) {
    rtn = EOF;
  }
  m_r = 0;
  m_w = 0;
  m_status = 0;
  return rtn;
}
//------------------------------------------------------------------------------
int StdioStream::fflush() {
  if ((m_status & (S_SWR | S_SRW)) && !(m_status & S_SRD)) {
    if (flushBuf() && StreamBaseFile::sync()) {
      return 0;
    }
  }
  return EOF;
}
//------------------------------------------------------------------------------
char* StdioStream::fgets(char* str, size_t num, size_t* len) {
  char* s = str;
  if (num-- == 0) {
    return 0;
  }
  while (num) {
    size_t n;
    if ((n = m_r) == 0) {
      if (!fillBuf()) {
        if (s == str) {
          return 0;
        }
        break;
      }
      n = m_r;
    }
    if (n > num) {
      n = num;
    }
    uint8_t* end = reinterpret_cast<uint8_t*>(memchr(m_p, '\n', n));
    if (end != 0) {
      n = ++end - m_p;
      memcpy(s, m_p, n);
      m_r -= n;
      m_p = end;
      s += n;
      break;
    }
    memcpy(s, m_p, n);
    m_r -= n;
    m_p += n;
    s += n;
    num -= n;
  }
  *s = 0;
  if (len) {
    *len = s - str;
  }
  return str;
}
//------------------------------------------------------------------------------
bool StdioStream::fopen(const char* path, const char* mode) {
  oflag_t oflag;
  uint8_t m;
  switch (*mode++) {
    case 'a':
      m = O_WRONLY;
      oflag = O_CREAT | O_APPEND;
      m_status = S_SWR;
      break;

    case 'r':
      m = O_RDONLY;
      oflag = 0;
      m_status = S_SRD;
      break;

    case 'w':
      m = O_WRONLY;
      oflag = O_CREAT | O_TRUNC;
      m_status = S_SWR;
      break;

    default:
      goto fail;
  }
  while (*mode) {
    switch (*mode++) {
      case '+':
        m_status = S_SRW;
        m = O_RDWR;
        break;

      case 'b':
        break;

      case 'x':
        oflag |= O_EXCL;
        break;

      default:
        goto fail;
    }
  }
  oflag |= m;
  if (!StreamBaseFile::open(path, oflag)) {
    goto fail;
  }
  m_r = 0;
  m_w = 0;
  m_p = m_buf;
  return true;

fail:
  m_status = 0;
  return false;
}
//------------------------------------------------------------------------------
int StdioStream::fputs(const char* str) {
  size_t len = strlen(str);
  return fwrite(str, 1, len) == len ? len : EOF;
}
//------------------------------------------------------------------------------
size_t StdioStream::fread(void* ptr, size_t size, size_t count) {
  uint8_t* dst = reinterpret_cast<uint8_t*>(ptr);
  size_t total = size * count;
  if (total == 0) {
    return 0;
  }
  size_t need = total;
  while (need > m_r) {
    memcpy(dst, m_p, m_r);
    dst += m_r;
    m_p += m_r;
    need -= m_r;
    if (!fillBuf()) {
      return (total - need) / size;
    }
  }
  memcpy(dst, m_p, need);
  m_r -= need;
  m_p += need;
  return count;
}
//------------------------------------------------------------------------------
int StdioStream::fseek(int32_t offset, int origin) {
  int32_t pos;
  if (m_status & S_SWR) {
    if (!flushBuf()) {
      goto fail;
    }
  }
  switch (origin) {
    case SEEK_CUR:
      pos = ftell();
      if (pos < 0) {
        goto fail;
      }
      pos += offset;
      if (!StreamBaseFile::seekCur(pos)) {
        goto fail;
      }
      break;

    case SEEK_SET:
      if (offset < 0) {
        goto fail;
      }
      if (!StreamBaseFile::seekSet(static_cast<uint32_t>(offset))) {
        goto fail;
      }
      break;

    case SEEK_END:
      if (!StreamBaseFile::seekEnd(offset)) {
        goto fail;
      }
      break;

    default:
      goto fail;
  }
  m_r = 0;
  m_p = m_buf;
  return 0;

fail:
  return EOF;
}
//------------------------------------------------------------------------------
int32_t StdioStream::ftell() {
  uint32_t pos = StreamBaseFile::curPosition();
  if (m_status & S_SRD) {
    if (m_r > pos) {
      return -1L;
    }
    pos -= m_r;
  } else if (m_status & S_SWR) {
    pos += m_p - m_buf;
  }
  return pos;
}
//------------------------------------------------------------------------------
size_t StdioStream::fwrite(const void* ptr, size_t size, size_t count) {
  return write(ptr, count * size) < 0 ? EOF : count;
}
//------------------------------------------------------------------------------
// allow shadow of rewind() in StreamBaseFile,
// cppcheck-suppress duplInheritedMember
int StdioStream::write(const void* buf, size_t count) {
  const uint8_t* src = static_cast<const uint8_t*>(buf);
  size_t todo = count;

  while (todo > m_w) {
    memcpy(m_p, src, m_w);
    m_p += m_w;
    src += m_w;
    todo -= m_w;
    if (!flushBuf()) {
      return EOF;
    }
  }
  memcpy(m_p, src, todo);
  m_p += todo;
  m_w -= todo;
  return count;
}
//------------------------------------------------------------------------------
#if (defined(ARDUINO) && ENABLE_ARDUINO_FEATURES) || defined(DOXYGEN)
size_t StdioStream::print(const __FlashStringHelper* str) {
#ifdef __AVR__
  PGM_P p = reinterpret_cast<PGM_P>(str);
  uint8_t c;
  while ((c = pgm_read_byte(p))) {
    if (putc(c) < 0) {
      return 0;
    }
    p++;
  }
  return p - reinterpret_cast<PGM_P>(str);
#else   // __AVR__
  return print(reinterpret_cast<const char*>(str));
#endif  // __AVR__
}
#endif  // (defined(ARDUINO) && ENABLE_ARDUINO_FEATURES) || defined(DOXYGEN)
//------------------------------------------------------------------------------
int StdioStream::printDec(float value, uint8_t prec) {
  char buf[24];
  const char* ptr = fmtDouble(buf + sizeof(buf), value, prec, false);
  return write(ptr, buf + sizeof(buf) - ptr);
}
//------------------------------------------------------------------------------
int StdioStream::printDec(signed char n) {
  if (n < 0) {
    if (fputc('-') < 0) {
      return -1;
    }
    n = -n;
  }
  return printDec((unsigned char)n);
}
//------------------------------------------------------------------------------
int StdioStream::printDec(int16_t n) {
  int s;
  uint8_t rtn = 0;
  if (n < 0) {
    if (fputc('-') < 0) {
      return -1;
    }
    n = -n;
    rtn++;
  }
  if ((s = printDec(static_cast<uint16_t>(n))) < 0) {
    return s;
  }
  return rtn;
}
//------------------------------------------------------------------------------
int StdioStream::printDec(uint16_t n) {
  char buf[5];
  const char* ptr = fmtBase10(buf + sizeof(buf), n);
  uint8_t len = buf + sizeof(buf) - ptr;
  return write(ptr, len);
}
//------------------------------------------------------------------------------
int StdioStream::printDec(int32_t n) {
  uint8_t s = 0;
  if (n < 0) {
    if (fputc('-') < 0) {
      return -1;
    }
    n = -n;
    s = 1;
  }
  int rtn = printDec(static_cast<uint32_t>(n));
  return rtn > 0 ? rtn + s : -1;
}
//------------------------------------------------------------------------------
int StdioStream::printDec(uint32_t n) {
  char buf[10];
  const char* ptr = fmtBase10(buf + sizeof(buf), n);
  uint8_t len = buf + sizeof(buf) - ptr;
  return write(ptr, len);
}
//------------------------------------------------------------------------------
int StdioStream::printHex(uint32_t n) {
  char buf[8];
  const char* ptr = fmtHex(buf + sizeof(buf), n);
  uint8_t len = buf + sizeof(buf) - ptr;
  return write(ptr, len);
}
//------------------------------------------------------------------------------
// allow shadow of rewind() in StreamBaseFile,
// cppcheck-suppress duplInheritedMember
bool StdioStream::rewind() {
  if (m_status & S_SWR) {
    if (!flushBuf()) {
      return false;
    }
  }
  StreamBaseFile::seekSet(0UL);
  m_r = 0;
  return true;
}
//------------------------------------------------------------------------------
int StdioStream::ungetc(int c) {
  // error if EOF.
  if (c == EOF) {
    return EOF;
  }
  // error if not reading.
  if ((m_status & S_SRD) == 0) {
    return EOF;
  }
  // error if no space.
  if (m_p == m_buf) {
    return EOF;
  }
  m_r++;
  m_status &= ~S_EOF;
  return *--m_p = (uint8_t)c;
}
//==============================================================================
// private
//------------------------------------------------------------------------------
int StdioStream::fillGet() {
  if (!fillBuf()) {
    return EOF;
  }
  m_r--;
  return *m_p++;
}
//------------------------------------------------------------------------------
// private
bool StdioStream::fillBuf() {
  if (!(m_status & S_SRD)) {  // check for S_ERR and S_EOF ??/////////////////
    if (!(m_status & S_SRW)) {
      m_status |= S_ERR;
      return false;
    }
    if (m_status & S_SWR) {
      if (!flushBuf()) {
        return false;
      }
      m_status &= ~S_SWR;
      m_status |= S_SRD;
      m_w = 0;
    }
  }
  m_p = m_buf + UNGETC_BUF_SIZE;
  int nr = StreamBaseFile::read(m_p, sizeof(m_buf) - UNGETC_BUF_SIZE);
  if (nr <= 0) {
    m_status |= nr < 0 ? S_ERR : S_EOF;
    m_r = 0;
    return false;
  }
  m_r = nr;
  return true;
}
//------------------------------------------------------------------------------
// private
bool StdioStream::flushBuf() {
  if (!(m_status & S_SWR)) {
    if (!(m_status & S_SRW)) {
      m_status |= S_ERR;
      return false;
    }
    m_status &= ~S_SRD;
    m_status |= S_SWR;
    m_r = 0;
    m_w = sizeof(m_buf);
    m_p = m_buf;
    return true;
  }
  uint8_t n = m_p - m_buf;
  m_p = m_buf;
  m_w = sizeof(m_buf);
  if (StreamBaseFile::write(m_buf, n) == n) {
    return true;
  }
  m_status |= S_ERR;
  return false;
}
//------------------------------------------------------------------------------
int StdioStream::flushPut(uint8_t c) {
  if (!flushBuf()) {
    return EOF;
  }
  m_w--;
  return *m_p++ = c;
}
