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
#ifndef DOXYGEN_SHOULD_SKIP_THIS
#include "FatFile.h"
bool FatFile::findSfn(uint8_t sfn[11], uint16_t* index) {
  uint16_t free = 0XFFFF;
  uint16_t idx;
  dir_t* dir;

  if (!isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  rewind();
  while (m_curPosition < m_fileSize) {
    idx = m_curPosition/32;
    if ((0XF & idx) == 0) {
      dir = readDirCache();
      if (!dir) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    } else {
      m_curPosition += 32;
      dir++;
    }
    // done if last entry
    if (dir->name[0] == DIR_NAME_FREE || dir->name[0] == DIR_NAME_DELETED) {
      if (free == 0XFFFF) free = idx;
      if (dir->name[0] == DIR_NAME_FREE) goto fail;
    } else if (DIR_IS_FILE_OR_SUBDIR(dir)) {
      if (!memcmp(sfn, dir->name, 11)) {
        *index = idx;
        return true;
      }
    }
  }

 fail:
  *index = free;
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::createLfn(FatFile* dirFile,
                        uint16_t bgnIndex, char* name, uint8_t oflag) {
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::findLfn(const char* name,
                      uint16_t *bgnIndex, uint16_t* endIndex) {
  bool fill;
  bool haveLong = false;
  uint8_t ndir;
  uint8_t test;
  uint16_t lfnBgn;
  uint16_t curIndex = 0XFFFF;
  uint16_t freeIndex = 0XFFFF;
  uint8_t freeCount = 0;
  uint8_t freeNeed;
  dir_t* dir;
  size_t len;
  bool is83;
  bool foundFree = false;
  const char* ptr;
  uint8_t name83[11];
  if (!isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  is83 = make83Name(name, name83, &ptr);
  for (len = 0; name[len] !=0 && name[len] != '/'; len++) {}
  // Assume LFN.
  freeNeed = (len + 12)/13 + 1;
  rewind();
  while (1) {
    curIndex = m_curPosition/32;
    if (m_curPosition == m_fileSize) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // read entry into cache
    dir = readDirCache();
    if (!dir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    #if 1
    if (dir->name[0] == DIR_NAME_DELETED) {
      if (!foundFree) {
        if (freeIndex == 0XFFFF) {
          freeIndex = curIndex;
          freeCount = 0;
        }
        if (++freeCount == freeNeed) {
          foundFree = true;
        }
      }
      continue;
    } else {
      if (!foundFree) freeIndex = 0XFFFF;
    }
    #endif
    if (dir->name[0] == DIR_NAME_FREE) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // skip empty slot or '.' or '..'
    if (dir->name[0] == DIR_NAME_DELETED || dir->name[0] == '.') {
      haveLong = false;
      continue;
    }
    if (DIR_IS_LONG_NAME(dir)) {
      ldir_t *ldir = reinterpret_cast<ldir_t*>(dir);
      if (!haveLong) {
        if ((ldir->ord & LDIR_ORD_LAST_LONG_ENTRY) == 0) continue;
        ndir = ldir->ord & ~LDIR_ORD_LAST_LONG_ENTRY;
        if (ndir < 1 || ndir > 20) continue;
        test = ldir->chksum;
        haveLong = true;
        lfnBgn = curIndex;
        fill = true;
      } else if (ldir->ord != --ndir || test != ldir->chksum) {
        haveLong = false;
        continue;
      }
      size_t nOff = 13*(ndir -1);
      for (int i = 12; i >= 0; i--) {
        uint16_t u = lfnChar(ldir, i);

        if (fill) {
          if (u == 0 || u == 0XFFFF) {
            continue;
          }
          if (len != (nOff + i +1)) {
            haveLong = false;
            break;
          }
          fill = false;
        }
        if (u > 255 || toupper(u) != toupper(name[nOff + i])) {
          haveLong = false;
          break;
        }
      }
    } else if (DIR_IS_FILE_OR_SUBDIR(dir)) {
      if (haveLong) {
        uint8_t sum = 0;
        for (uint8_t i = 0; i < 11; i++) {
           sum = (((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + dir->name[i];
        }
        if (sum != test || ndir != 1) {
          haveLong = false;
        }
      }
      if (haveLong) goto done;
      if (is83 && !memcmp(dir->name, name83, sizeof(name83))) {
        goto done;
      }
    } else {
      haveLong = false;
    }
  }

 done:
  *bgnIndex = haveLong ? lfnBgn : curIndex;
  *endIndex = curIndex;
  return true;

 fail:
  *bgnIndex = foundFree ? freeIndex : curIndex;
  return false;
}
//------------------------------------------------------------------------------
int FatFile::openNextLFN(FatFile* dirFile,
                         char* name, size_t size, uint8_t oflag) {
  bool fill;
  bool haveLong = false;
  size_t lfnIn;
  uint8_t ndir;
  uint8_t test;
  dir_t* dir;
  uint16_t index;
  int rtn;

  // Check for valid directory and file is not open.
  if (!dirFile->isDir() || isOpen() || size < 13) {
    DBG_FAIL_MACRO;
    goto fail;
  }

  while (1) {
    // Check for EOF.
    if (dirFile->curPosition() == dirFile->fileSize()) goto done;
    index = dirFile->curPosition()/32;
    // read entry into cache
    dir = dirFile->readDirCache();
    if (!dir) {
      DBG_FAIL_MACRO;
      goto fail;
    }

    // done if last entry
    if (dir->name[0] == DIR_NAME_FREE) {
      DBG_FAIL_MACRO;
      return 0;
    }
    // skip empty slot or '.' or '..'
    if (dir->name[0] == DIR_NAME_DELETED || dir->name[0] == '.') {
      haveLong = false;
      continue;
    }
    if (DIR_IS_LONG_NAME(dir)) {
      ldir_t *ldir = reinterpret_cast<ldir_t*>(dir);
      if (!haveLong) {
        if ((ldir->ord & LDIR_ORD_LAST_LONG_ENTRY) == 0) continue;
        ndir = ldir->ord & ~LDIR_ORD_LAST_LONG_ENTRY;
        if (ndir < 1 || ndir > 20) continue;
        test = ldir->chksum;
        haveLong = true;
        rtn = 0;
        fill = true;
        lfnIn = 13*ndir;
      } else if (ldir->ord != --ndir || test != ldir->chksum) {
        haveLong = false;
        continue;
      }
      for (int i = 12; i >= 0; i--) {
        uint16_t u = lfnChar(ldir, i);
        if (fill) {
          if (rtn == 0 && u != 0 && u != 0XFFFF) rtn = lfnIn;
          if (u == 0 || u == 0XFFFF || lfnIn >= size) {
            lfnIn--;
            continue;
          }
          name[lfnIn] = 0;
          fill = false;
        }
        if (lfnIn == 0 || u > 255) {
          haveLong = false;
          break;
        }
        name[--lfnIn] = u;
      }
    } else if (DIR_IS_FILE_OR_SUBDIR(dir)) {
      if (haveLong) {
        uint8_t sum = 0;
        for (uint8_t i = 0; i < 11; i++) {
           sum = (((sum & 1) << 7) | ((sum & 0xfe) >> 1)) + dir->name[i];
        }
        if (sum != test || ndir != 1) {
          haveLong = false;
        }
      }
      if (!haveLong) {
        rtn = dirName(dir, name);
        if (dir->reservedNT) {
          uint8_t lowerTest = 0X08;
          for (char *ptr = name; *ptr; ptr++) {
            if (*ptr == '.') {
              lowerTest = 0X10;
              continue;
            }
            if (dir->reservedNT & lowerTest) {
              *ptr = tolower(*ptr);
            }
          }
        }
      }
      if (!openCachedEntry(dirFile, index, oflag)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      return rtn;
    } else {
      haveLong = false;
    }
  }

 done:
  return 0;

 fail:
  return -1;
}
#endif  // DOXYGEN_SHOULD_SKIP_THIS
