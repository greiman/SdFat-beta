/* FatLib Library
 * Copyright (C) 2012 by William Greiman
 *
 * This file is part of the FatLib Library
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
 * along with the FatLib Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#include <math.h>
#include "FatFile.h"
#include "FmtNumber.h"
//------------------------------------------------------------------------------
// print uint8_t with width 2
static void print2u(print_t* pr, uint8_t v) {
  char c0 = '?';
  char c1 = '?';
  if (v < 100) {
    c1 = v/10;
    c0 = v - 10*c1 + '0';
    c1 += '0';
  }
  pr->write(c1);
  pr->write(c0);
}
//------------------------------------------------------------------------------
static void printU32(print_t* pr, uint32_t v) {
  char buf[11];
  char* ptr = buf + sizeof(buf);
  *--ptr = 0;
  pr->write(fmtDec(v, ptr));
}
//------------------------------------------------------------------------------
void FatFile::ls(print_t* pr, uint8_t flags, uint8_t indent) {
  FatFile file;
  rewind();
  while (file.openNext(this, O_READ)) {
    // indent for dir level
    if (!file.isHidden() && !(flags & LS_A)) {
      for (uint8_t i = 0; i < indent; i++) {
        pr->write(' ');
      }
      if (flags & LS_DATE) {
        file.printModifyDateTime(pr);
        pr->write(' ');
      }
      if (flags & LS_SIZE) {
        file.printFileSize(pr);
        pr->write(' ');
      }
      file.printName(pr);
      if (file.isDir()) {
        pr->write('/');
      }
      pr->write('\r');
      pr->write('\n');
      if ((flags & LS_R) && file.isDir()) {
        file.ls(pr, flags, indent + 2);
      }
    }
    file.close();
  }
}
//------------------------------------------------------------------------------
bool FatFile::printCreateDateTime(print_t* pr) {
  dir_t dir;
  if (!dirEntry(&dir)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  printFatDate(pr, dir.creationDate);
  pr->write(' ');
  printFatTime(pr, dir.creationTime);
  return true;

fail:
  return false;
}
//------------------------------------------------------------------------------
void FatFile::printFatDate(print_t* pr, uint16_t fatDate) {
  printU32(pr, FAT_YEAR(fatDate));
  pr->write('-');
  print2u(pr, FAT_MONTH(fatDate));
  pr->write('-');
  print2u(pr, FAT_DAY(fatDate));
}
//------------------------------------------------------------------------------
void FatFile::printFatTime(print_t* pr, uint16_t fatTime) {
  print2u(pr, FAT_HOUR(fatTime));
  pr->write(':');
  print2u(pr, FAT_MINUTE(fatTime));
  pr->write(':');
  print2u(pr, FAT_SECOND(fatTime));
}
//------------------------------------------------------------------------------
/** Template for FatFile::printField() */
template <typename Type>
static int printFieldT(FatFile* file, char sign, Type value, char term) {
  char buf[3*sizeof(Type) + 3];
  char* str = &buf[sizeof(buf)];

  if (term) {
    *--str = term;
    if (term == '\n') {
      *--str = '\r';
    }
  }
#ifdef OLD_FMT
  do {
    Type m = value;
    value /= 10;
    *--str = '0' + m - 10*value;
  } while (value);
#else  // OLD_FMT
  str = fmtDec(value, str);
#endif  // OLD_FMT
  if (sign) {
    *--str = sign;
  }
  return file->write(str, &buf[sizeof(buf)] - str);
}
//------------------------------------------------------------------------------

int FatFile::printField(float value, char term, uint8_t prec) {
  char buf[24];
  char* str = &buf[sizeof(buf)];
  if (term) {
    *--str = term;
    if (term == '\n') {
      *--str = '\r';
    }
  }
  str = fmtFloat(value, str, prec);
  return write(str, buf + sizeof(buf) - str);
}
//------------------------------------------------------------------------------
int FatFile::printField(uint16_t value, char term) {
  return printFieldT(this, 0, value, term);
}
//------------------------------------------------------------------------------
int FatFile::printField(int16_t value, char term) {
  char sign = 0;
  if (value < 0) {
    sign = '-';
    value = -value;
  }
  return printFieldT(this, sign, (uint16_t)value, term);
}
//------------------------------------------------------------------------------
int FatFile::printField(uint32_t value, char term) {
  return printFieldT(this, 0, value, term);
}
//------------------------------------------------------------------------------
int FatFile::printField(int32_t value, char term) {
  char sign = 0;
  if (value < 0) {
    sign = '-';
    value = -value;
  }
  return printFieldT(this, sign, (uint32_t)value, term);
}
//------------------------------------------------------------------------------
bool FatFile::printModifyDateTime(print_t* pr) {
  dir_t dir;
  if (!dirEntry(&dir)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  printFatDate(pr, dir.lastWriteDate);
  pr->write(' ');
  printFatTime(pr, dir.lastWriteTime);
  return true;

fail:
  return false;
}

//------------------------------------------------------------------------------
size_t FatFile::printFileSize(print_t* pr) {
  char buf[11];
  char *ptr = buf + sizeof(buf);
  *--ptr = 0;
  ptr = fmtDec(fileSize(), ptr);
  while (ptr > buf) {
    *--ptr = ' ';
  }
  return pr->write(buf);
}
