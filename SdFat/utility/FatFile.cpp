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
#include "FatFile.h"
#include "FatFileSystem.h"
//------------------------------------------------------------------------------
// Pointer to cwd directory.
FatFile* FatFile::m_cwd = 0;
// Callback function for date/time.
void (*FatFile::m_dateTime)(uint16_t* date, uint16_t* time) = 0;
//------------------------------------------------------------------------------
// Add a cluster to a file.
bool FatFile::addCluster() {
  m_flags |= F_FILE_DIR_DIRTY;
  return m_vol->allocateCluster(m_curCluster, &m_curCluster);
}
//------------------------------------------------------------------------------
// Add a cluster to a directory file and zero the cluster.
// Return with first block of cluster in the cache.
bool FatFile::addDirCluster() {
  uint32_t block;
  cache_t* pc;

  // max folder size
  if (m_fileSize/sizeof(dir_t) >= 0XFFFF) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!addCluster()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  block = m_vol->clusterStartBlock(m_curCluster);
  pc = m_vol->cacheFetchData(block, FatCache::CACHE_RESERVE_FOR_WRITE);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  memset(pc, 0, 512);
  // zero rest of clusters
  for (uint8_t i = 1; i < m_vol->blocksPerCluster(); i++) {
    if (!m_vol->writeBlock(block + i, pc->data)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // Increase directory file size by cluster size.
  m_fileSize += 512UL*m_vol->blocksPerCluster();

  // Set position to EOF to avoid inconsistent curCluster/curPosition.
  m_curPosition = m_fileSize;
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
// cache a file's directory entry
// return pointer to cached entry or null for failure
dir_t* FatFile::cacheDirEntry(uint8_t action) {
  cache_t* pc;
  pc = m_vol->cacheFetchData(m_dirBlock, action);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return pc->dir + (m_dirIndex & 0XF);

 fail:
  return 0;
}
//------------------------------------------------------------------------------
bool FatFile::close() {
  bool rtn = sync();
  m_attr = FILE_ATTR_CLOSED;
  return rtn;
}
//------------------------------------------------------------------------------
bool FatFile::contiguousRange(uint32_t* bgnBlock, uint32_t* endBlock) {
  // error if no blocks
  if (m_firstCluster == 0) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  for (uint32_t c = m_firstCluster; ; c++) {
    uint32_t next;
    if (!m_vol->fatGet(c, &next)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // check for contiguous
    if (next != (c + 1)) {
      // error if not end of chain
      if (!m_vol->isEOC(next)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      *bgnBlock = m_vol->clusterStartBlock(m_firstCluster);
      *endBlock = m_vol->clusterStartBlock(c)
                  + m_vol->blocksPerCluster() - 1;
      return true;
    }
  }

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::createContiguous(FatFile* dirFile,
        const char* path, uint32_t size) {
  uint32_t count;
  // don't allow zero length file
  if (size == 0) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (!open(dirFile, path, O_CREAT | O_EXCL | O_RDWR)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // calculate number of clusters needed
  count = ((size - 1) >> (m_vol->clusterSizeShift() + 9)) + 1;

  // allocate clusters
  if (!m_vol->allocContiguous(count, &m_firstCluster)) {
    remove();
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_fileSize = size;

  // insure sync() will update dir entry
  m_flags |= F_FILE_DIR_DIRTY;

  return sync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::dirEntry(dir_t* dst) {
  dir_t* dir;
  // Make sure fields on device are correct.
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // read entry
  dir = cacheDirEntry(FatCache::CACHE_FOR_READ);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // copy to caller's struct
  memcpy(dst, dir, sizeof(dir_t));
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
uint8_t FatFile::dirName(const dir_t* dir, char* name) {
  uint8_t j = 0;
  for (uint8_t i = 0; i < 11; i++) {
    if (dir->name[i] == ' ') continue;
    if (i == 8) name[j++] = '.';
    name[j++] = dir->name[i];
  }
  name[j] = 0;
  return j;
}
//------------------------------------------------------------------------------
int16_t FatFile::fgets(char* str, int16_t num, char* delim) {
  char ch;
  int16_t n = 0;
  int16_t r = -1;
  while ((n + 1) < num && (r = read(&ch, 1)) == 1) {
    // delete CR
    if (ch == '\r') continue;
    str[n++] = ch;
    if (!delim) {
      if (ch == '\n') break;
    } else {
      if (strchr(delim, ch)) break;
    }
  }
  if (r < 0) {
    // read error
    return -1;
  }
  str[n] = '\0';
  return n;
}
//------------------------------------------------------------------------------
bool FatFile::getFilename(char* name) {
  dir_t* dir;
  if (!isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isRoot()) {
    name[0] = '/';
    name[1] = '\0';
    return true;
  }
  // cache entry
  dir = cacheDirEntry(FatCache::CACHE_FOR_READ);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // format name
  dirName(dir, name);
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
void FatFile::getpos(FatPos_t* pos) {
  pos->position = m_curPosition;
  pos->cluster = m_curCluster;
}
//------------------------------------------------------------------------------
// format directory name field from a 8.3 name string
bool FatFile::make83Name(const char* str, uint8_t* name, const char** ptr) {
  uint8_t c;
  uint8_t n = 7;  // max index for part before dot
  uint8_t i = 0;
  // blank fill name and extension
  while (i < 11) name[i++] = ' ';
  i = 0;
  while (*str != '\0' && *str != '/') {
    c = *str++;
    if (c == '.') {
      if (n == 10) {
        // only one dot allowed
        DBG_FAIL_MACRO;
        goto fail;
      }
      n = 10;  // max index for full 8.3 name
      i = 8;   // place for extension
    } else {
      // illegal FAT characters
#ifdef __AVR__
      // store chars in flash
      PGM_P p = PSTR("|<>^+=?/[];,*\"\\");
      uint8_t b;
      while ((b = pgm_read_byte(p++))) if (b == c) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#else  // __AVR__
      // store chars in RAM
      if (strchr("|<>^+=?/[];,*\"\\", c)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#endif  // __AVR__

      // check size and only allow ASCII printable characters
      if (i > n || c < 0X21 || c > 0X7E) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      // only upper case allowed in 8.3 names - convert lower to upper
      name[i++] = c < 'a' || c > 'z' ?  c : c + ('A' - 'a');
    }
  }
  *ptr = str;
  // must have a file name, extension is optional
  return name[0] != ' ';

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::mkdir(FatFile* parent, const char* path, bool pFlag) {
  uint8_t dname[11];
  FatFile tmpDir;

  if (isOpen() || !parent->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (*path == '/') {
    while (*path == '/') path++;
    if (!parent->isRoot()) {
      if (!tmpDir.openRoot(parent->m_vol)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      parent = &tmpDir;
    }
  }
  while (1) {
    if (!make83Name(path, dname, &path)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    while (*path == '/') path++;
    if (!*path) break;
    if (!open(parent, dname, O_READ)) {
      if (!pFlag || !mkdir(parent, dname)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    tmpDir = *this;
    parent = &tmpDir;
    close();
  }
  return mkdir(parent, dname);

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::mkdir(FatFile* parent, const uint8_t dname[11]) {
  uint32_t block;
  dir_t dot;
  dir_t* dir;
  cache_t* pc;

  if (!parent->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // create a normal file
  if (!open(parent, dname, O_CREAT | O_EXCL | O_RDWR)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // convert file to directory
  m_flags = O_READ;
  m_attr = FILE_ATTR_IS_OPEN | FILE_ATTR_SUBDIR;

  // allocate and zero first cluster
  if (!addDirCluster()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_firstCluster = m_curCluster;
  // Set to start of dir
  rewind();
  // force entry to device
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // cache entry - should already be in cache due to sync() call
  dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // change directory entry  attribute
  dir->attributes = DIR_ATT_DIRECTORY;

  // make entry for '.'
  memcpy(&dot, dir, sizeof(dot));
  dot.name[0] = '.';
  for (uint8_t i = 1; i < 11; i++) dot.name[i] = ' ';

  // cache block for '.'  and '..'
  block = m_vol->clusterStartBlock(m_firstCluster);
  pc = m_vol->cacheFetchData(block, FatCache::CACHE_FOR_WRITE);
  if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // copy '.' to block
  memcpy(&pc->dir[0], &dot, sizeof(dot));
  // make entry for '..'
  dot.name[1] = '.';
  if (parent->isRoot()) {
    dot.firstClusterLow = 0;
    dot.firstClusterHigh = 0;
  } else {
    dot.firstClusterLow = parent->m_firstCluster & 0XFFFF;
    dot.firstClusterHigh = parent->m_firstCluster >> 16;
  }
  // copy '..' to block
  memcpy(&pc->dir[1], &dot, sizeof(dot));
  // write first block
  return m_vol->cacheSync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::open(FatFileSystem* fs, const char* path, uint8_t oflag) {
  return open(fs->vwd(), path, oflag);
}
//------------------------------------------------------------------------------
bool FatFile::open(FatFile* dirFile, const char* path, uint8_t oflag) {
  FatFile tmpDir;
  uint8_t dname[11];

  // error if already open
  if (isOpen() || !dirFile->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (*path == '/') {
    while (*path == '/') path++;
    if (*path == 0) return openRoot(dirFile->m_vol);
    if (!dirFile->isRoot()) {
      if (!tmpDir.openRoot(dirFile->m_vol)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      dirFile = &tmpDir;
    }
  }
  while (1) {
    if (!make83Name(path, dname, &path)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    while (*path == '/') path++;
    if (*path == 0) break;
    if (!open(dirFile, dname, O_READ) || !isDir()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    tmpDir = *this;
    dirFile = &tmpDir;
    close();
  }
  return open(dirFile, dname, oflag);

 fail:
  return false;
}
//------------------------------------------------------------------------------
// open with filename in dname
bool FatFile::open(FatFile* dirFile,
                      const uint8_t dname[11], uint8_t oflag) {
  bool emptyFound = false;
  bool fileFound = false;
  uint16_t emptyIndex;
  uint16_t index = 0;
  dir_t* dir;

  dirFile->rewind();
  while (1) {
    if (!emptyFound) emptyIndex = index;
    if (dirFile->m_curPosition >= dirFile->m_fileSize) break;

    if ((0XF & index) == 0) {
      dir = dirFile->readDirCache();
      if (!dir) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    } else {
      dirFile->m_curPosition += 32;
      dir++;
    }
    // done if last entry
    if (dir->name[0] == DIR_NAME_FREE || dir->name[0] == DIR_NAME_DELETED) {
      emptyFound = true;
      if (dir->name[0] == DIR_NAME_FREE) break;
    } else if (DIR_IS_FILE_OR_SUBDIR(dir)) {
      if (!memcmp(dname, dir->name, 11)) {
       fileFound = true;
       break;
      }
    }
    index++;
  }

  if (fileFound) {
    // don't open existing file if O_EXCL
    if (oflag & O_EXCL) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else {
    // don't create unless O_CREAT and O_WRITE
    if (!(oflag & O_CREAT) || !(oflag & O_WRITE)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!emptyFound) {
      if (dirFile->isRootFixed()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      if (!dirFile->addDirCluster()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    index = emptyIndex;
    dirFile->seekSet(32UL*index);
    dir = dirFile->readDirCache();
    if (!dir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // initialize as empty file
    memset(dir, 0, sizeof(dir_t));
    memcpy(dir->name, dname, 11);

    // set timestamps
    if (m_dateTime) {
      // call user date/time function
      m_dateTime(&dir->creationDate, &dir->creationTime);
    } else {
      // use default date/time
      dir->creationDate = FAT_DEFAULT_DATE;
      dir->creationTime = FAT_DEFAULT_TIME;
    }
    dir->lastAccessDate = dir->creationDate;
    dir->lastWriteDate = dir->creationDate;
    dir->lastWriteTime = dir->creationTime;

    // Force write of entry to device.
    dirFile->m_vol->cacheDirty();
  }
  // open entry in cache
  return openCachedEntry(dirFile, index, oflag);

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::open(FatFile* dirFile, uint16_t index, uint8_t oflag) {
  dir_t* dir;

  // error if already open
  if (isOpen() || !dirFile->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // don't open existing file if O_EXCL - user call error
  if (oflag & O_EXCL) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // seek to location of entry
  if (!dirFile->seekSet(32UL * index)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // read entry into cache
  dir = dirFile->readDirCache();
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // error if empty slot or '.' or '..'
  if (dir->name[0] == DIR_NAME_FREE ||
      dir->name[0] == DIR_NAME_DELETED || dir->name[0] == '.') {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // open cached entry
  return openCachedEntry(dirFile, index, oflag);

 fail:
  return false;
}
//------------------------------------------------------------------------------
// open a cached directory entry. Assumes m_vol is initialized
bool FatFile::openCachedEntry(FatFile* dirFile,
                                 uint16_t dirIndex, uint8_t oflag) {
  // location of entry in cache
  m_vol = dirFile->m_vol;
  dir_t* dir = &m_vol->cacheAddress()->dir[0XF & dirIndex];

  // Must be file or subdirectory.
  if (!DIR_IS_FILE_OR_SUBDIR(dir)) {
    DBG_FAIL_MACRO;
     goto fail;
  }
  m_attr =  FILE_ATTR_IS_OPEN | (dir->attributes & FILE_ATTR_COPY);

  // Write or truncate is an error for a directory or read-only file
  if (isSubDir() || isReadOnly()) {
    if (oflag & (O_WRITE | O_TRUNC)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // remember location of directory entry on device
  m_dirFirstCluster = dirFile->m_firstCluster;
  m_dirBlock = m_vol->cacheBlockNumber();
  m_dirIndex = dirIndex;

  // copy first cluster number for directory fields
  m_firstCluster = (uint32_t)dir->firstClusterHigh << 16;
  m_firstCluster |= dir->firstClusterLow;

  // Set file size
  if (isSubDir()) {
    if (!setDirSize()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else {
    m_fileSize = dir->fileSize;
  }
  // save open flags for read/write
  m_flags = oflag & F_OFLAG;

  // set to start of file
  m_curCluster = 0;
  m_curPosition = 0;
  if ((oflag & O_TRUNC) && !truncate(0)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return oflag & O_AT_END ? seekEnd(0) : true;

 fail:
  m_attr = FILE_ATTR_CLOSED;
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::openNext(FatFile* dirFile, uint8_t oflag) {
  uint16_t index;
  // Check for valid directory and file is not open.
  if (!dirFile->isDir() || isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  while (1) {
    // Check for EOF.
    if (dirFile->curPosition() == dirFile->fileSize()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // read entry into cache
    index = dirFile->curPosition()/32;
    dir_t* dir = dirFile->readDirCache();
    if (!dir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // done if last entry
    if (dir->name[0] == DIR_NAME_FREE) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // must be file or dir
    // skip empty slot or '.' or '..'
    if (DIR_IS_FILE_OR_SUBDIR(dir) && dir->name[0] != '.' &&
        dir->name[0] != DIR_NAME_DELETED) {
      return openCachedEntry(dirFile, index, oflag);
    }
  }

 fail:
  return false;
}
//------------------------------------------------------------------------------
/** Open a directory's parent directory.
 *
 * \param[in] dir Parent of this directory will be opened.  Must not be root.
 *
 * \return The value one, true, is returned for success and
 * the value zero, false, is returned for failure.
 */
bool FatFile::openParent(FatFile* dirFile) {
  dir_t entry;
  dir_t* dir;
  FatFile file;
  uint32_t c;
  uint32_t cluster;
  uint32_t lbn;
  cache_t* pc;
  // error if already open or dir is root or dir is not a directory
  if (isOpen() || dirFile->isRoot() || !dirFile->isDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_vol = dirFile->m_vol;
  // position to '..'
  if (!dirFile->seekSet(32)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // read '..' entry
  if (dirFile->read(&entry, sizeof(entry)) != 32) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // verify it is '..'
  if (entry.name[0] != '.' || entry.name[1] != '.') {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // start cluster for '..'
  cluster = entry.firstClusterLow;
  cluster |= (uint32_t)entry.firstClusterHigh << 16;
  if (cluster == 0) return openRoot(m_vol);
  // start block for '..'
  lbn = m_vol->clusterStartBlock(cluster);
  // first block of parent dir
    pc = m_vol->cacheFetchData(lbn, FatCache::CACHE_FOR_READ);
    if (!pc) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  dir = &pc->dir[1];
  // verify name for '../..'
  if (dir->name[0] != '.' || dir->name[1] != '.') {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // '..' is pointer to first cluster of parent. open '../..' to find parent
  if (dir->firstClusterHigh == 0 && dir->firstClusterLow == 0) {
    if (!file.openRoot(dirFile->volume())) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else {
    if (!file.openCachedEntry(dirFile, 1, O_READ)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // search for parent in '../..'
  do {
    if (file.readDir(&entry) != 32) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    c = entry.firstClusterLow;
    c |= (uint32_t)entry.firstClusterHigh << 16;
  } while (c != cluster);
  // open parent
  return open(&file, file.curPosition()/32 - 1, O_READ);

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::openRoot(FatVolume* vol) {
  // error if file is already open
  if (isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  m_vol = vol;
  if (vol->fatType() == 16 || (FAT12_SUPPORT && vol->fatType() == 12)) {
    m_attr = FILE_ATTR_IS_OPEN | FILE_ATTR_ROOT_FIXED;
    m_firstCluster = 0;
    m_fileSize = 32 * vol->rootDirEntryCount();
  } else if (vol->fatType() == 32) {
    m_attr = FILE_ATTR_IS_OPEN | FILE_ATTR_ROOT32;
    m_firstCluster = vol->rootDirStart();
    if (!setDirSize()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } else {
    // volume is not initialized, invalid, or FAT12 without support
    DBG_FAIL_MACRO;
    goto fail;
  }
  // read only
  m_flags = O_READ;

  // set to start of file
  m_curCluster = 0;
  m_curPosition = 0;

  // root has no directory entry
  m_dirBlock = 0;
  m_dirIndex = 0;
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
int FatFile::peek() {
  FatPos_t pos;
  getpos(&pos);
  int c = read();
  if (c >= 0) setpos(&pos);
  return c;
}
//------------------------------------------------------------------------------
int FatFile::read(void* buf, size_t nbyte) {
  uint8_t blockOfCluster;
  uint8_t* dst = reinterpret_cast<uint8_t*>(buf);
  uint16_t offset;
  size_t toRead;
  uint32_t block;  // raw device block number
  cache_t* pc;

  // error if not open or write only
  if (!isOpen() || !(m_flags & O_READ)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // max bytes left in file
  if (nbyte >= (m_fileSize - m_curPosition)) {
    nbyte = m_fileSize - m_curPosition;
  }
  // amount left to read
  toRead = nbyte;
  while (toRead > 0) {
    size_t n;
    offset = m_curPosition & 0X1FF;  // offset in block
    blockOfCluster = m_vol->blockOfCluster(m_curPosition);
    if (isRootFixed()) {
      block = m_vol->rootDirStart() + (m_curPosition >> 9);
    } else {
      if (offset == 0 && blockOfCluster == 0) {
        // start of new cluster
        if (m_curPosition == 0) {
          // use first cluster in file
          m_curCluster = m_firstCluster;
        } else {
          // get next cluster from FAT
          if (!m_vol->fatGet(m_curCluster, &m_curCluster)) {
            DBG_FAIL_MACRO;
            goto fail;
          }
        }
      }
      block = m_vol->clusterStartBlock(m_curCluster) + blockOfCluster;
    }
    if (offset != 0 || toRead < 512 || block == m_vol->cacheBlockNumber()) {
      // amount to be read from current block
      n = 512 - offset;
      if (n > toRead) n = toRead;
      // read block to cache and copy data to caller
      pc = m_vol->cacheFetchData(block, FatCache::CACHE_FOR_READ);
      if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      uint8_t* src = pc->data + offset;
      memcpy(dst, src, n);
#if USE_MULTI_BLOCK_IO
    } else if (toRead >= 1024) {
      uint8_t nb = toRead >> 9;
      if (!isRootFixed()) {
        uint8_t mb = m_vol->blocksPerCluster() - blockOfCluster;
        if (mb < nb) nb = mb;
      }
      n = 512*nb;
      if (m_vol->cacheBlockNumber() <= block
        && block < (m_vol->cacheBlockNumber() + nb)) {
        // flush cache if a block is in the cache
        if (!m_vol->cacheSync()) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
      if (!m_vol->readBlocks(block, dst, nb)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#endif  // USE_MULTI_BLOCK_IO
    } else {
      // read single block
      n = 512;
      if (!m_vol->readBlock(block, dst)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    dst += n;
    m_curPosition += n;
    toRead -= n;
  }
  return nbyte;

 fail:
  return -1;
}
//------------------------------------------------------------------------------
int8_t FatFile::readDir(dir_t* dir) {
  int16_t n;
  // if not a directory file or miss-positioned return an error
  if (!isDir() || (0X1F & m_curPosition)) return -1;

  while (1) {
    n = read(dir, sizeof(dir_t));
    if (n != sizeof(dir_t)) return n == 0 ? 0 : -1;
    // last entry if DIR_NAME_FREE
    if (dir->name[0] == DIR_NAME_FREE) return 0;
    // skip empty entries and entry for .  and ..
    if (dir->name[0] == DIR_NAME_DELETED || dir->name[0] == '.') continue;
    // return if normal file or subdirectory
    if (DIR_IS_FILE_OR_SUBDIR(dir)) return n;
  }
}
//------------------------------------------------------------------------------
// Read next directory entry into the cache
// Assumes file is correctly positioned
dir_t* FatFile::readDirCache() {
  uint8_t i;
  // index of entry in cache
  i = (m_curPosition >> 5) & 0XF;

  // use read to locate and cache block
  if (read() < 0) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // advance to next entry
  m_curPosition += 31;

  // return pointer to entry
  return m_vol->cacheAddress()->dir + i;

 fail:
  return 0;
}
//------------------------------------------------------------------------------
bool FatFile::remove() {
  dir_t* dir;
  // Free any clusters - will fail if read-only or directory.
  if (!truncate(0)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Cache directory entry.
  dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Mark entry deleted.
  dir->name[0] = DIR_NAME_DELETED;

  // Set this file closed.
  m_attr = FILE_ATTR_CLOSED;

  // Write entry to device.
  return m_vol->cacheSync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::remove(FatFile* dirFile, const char* path) {
  FatFile file;
  if (!file.open(dirFile, path, O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  return file.remove();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::rename(FatFile* dirFile, const char* newPath) {
  dir_t entry;
  uint32_t dirCluster = 0;
  FatFile file;
  cache_t* pc;
  dir_t* dir;

  // Must be an open file or subdirectory.
  if (!(isFile() || isSubDir())) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // Can't move file to new volume.
  if (m_vol != dirFile->m_vol) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // sync() and cache directory entry
  sync();
  dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // save directory entry
  memcpy(&entry, dir, sizeof(entry));

  // mark entry deleted
  dir->name[0] = DIR_NAME_DELETED;

  // make directory entry for new path
  if (isFile()) {
    if (!file.open(dirFile, newPath, O_CREAT | O_EXCL | O_WRITE)) {
      goto restore;
    }
  } else {
    // don't create missing path prefix components
    if (!file.mkdir(dirFile, newPath, false)) {
      goto restore;
    }
    // save cluster containing new dot dot
    dirCluster = file.m_firstCluster;
  }
  // change to new directory entry
  m_dirBlock = file.m_dirBlock;
  m_dirIndex = file.m_dirIndex;

  // mark closed to avoid possible destructor close call
  file.m_attr = FILE_ATTR_CLOSED;
  // cache new directory entry
  dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // copy all but name field to new directory entry
  memcpy(&dir->attributes, &entry.attributes,
         sizeof(entry) - sizeof(dir->name));

  // update dot dot if directory
  if (dirCluster) {
    // get new dot dot
    uint32_t block = m_vol->clusterStartBlock(dirCluster);
    pc = m_vol->cacheFetchData(block, FatCache::CACHE_FOR_READ);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    memcpy(&entry, &pc->dir[1], sizeof(entry));

    // free unused cluster
    if (!m_vol->freeChain(dirCluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // store new dot dot
    block = m_vol->clusterStartBlock(m_firstCluster);
    pc = m_vol->cacheFetchData(block, FatCache::CACHE_FOR_WRITE);
    if (!pc) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    memcpy(&pc->dir[1], &entry, sizeof(entry));
  }
  return m_vol->cacheSync();

 restore:
  dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // restore entry
  dir->name[0] = entry.name[0];
  m_vol->cacheSync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::rmdir() {
  // must be open subdirectory
  if (!isSubDir()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  rewind();

  // make sure directory is empty
  while (m_curPosition < m_fileSize) {
    dir_t* dir = readDirCache();
    if (!dir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // done if past last used entry
    if (dir->name[0] == DIR_NAME_FREE) break;
    // skip empty slot, '.' or '..'
    if (dir->name[0] == DIR_NAME_DELETED || dir->name[0] == '.') continue;
    // error not empty
    if (DIR_IS_FILE_OR_SUBDIR(dir)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // convert empty directory to normal file for remove
  m_attr = FILE_ATTR_IS_OPEN;
  m_flags |= O_WRITE;
  return remove();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::rmRfStar() {
  uint16_t index;
  FatFile f;
  rewind();
  while (m_curPosition < m_fileSize) {
    // remember position
    index = m_curPosition/32;

    dir_t* dir = readDirCache();
    if (!dir) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // done if past last entry
    if (dir->name[0] == DIR_NAME_FREE) break;

    // skip empty slot or '.' or '..'
    if (dir->name[0] == DIR_NAME_DELETED || dir->name[0] == '.') continue;

    // skip if part of long file name or volume label in root
    if (!DIR_IS_FILE_OR_SUBDIR(dir)) continue;

    if (!f.open(this, index, O_READ)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (f.isSubDir()) {
      // recursively delete
      if (!f.rmRfStar()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    } else {
      // ignore read-only
      f.m_flags |= O_WRITE;
      if (!f.remove()) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    // position to next entry if required
    if (m_curPosition != (32UL*(index + 1))) {
      if (!seekSet(32UL*(index + 1))) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
  }
  // don't try to delete root
  if (!isRoot()) {
    if (!rmdir()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::seekSet(uint32_t pos) {
  uint32_t nCur;
  uint32_t nNew;
  // error if file not open or seek past end of file
  if (!isOpen() || pos > m_fileSize) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (isRootFixed()) {
    m_curPosition = pos;
    goto done;
  }
  if (pos == 0) {
    // set position to start of file
    m_curCluster = 0;
    m_curPosition = 0;
    goto done;
  }
  // calculate cluster index for cur and new position
  nCur = (m_curPosition - 1) >> (m_vol->clusterSizeShift() + 9);
  nNew = (pos - 1) >> (m_vol->clusterSizeShift() + 9);

  if (nNew < nCur || m_curPosition == 0) {
    // must follow chain from first cluster
    m_curCluster = m_firstCluster;
  } else {
    // advance from curPosition
    nNew -= nCur;
  }
  while (nNew--) {
    if (!m_vol->fatGet(m_curCluster, &m_curCluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  m_curPosition = pos;

 done:
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
// set m_fileSize for a directory
bool FatFile::setDirSize() {
  uint16_t s = 0;
  uint32_t cluster = m_firstCluster;
  do {
    if (!m_vol->fatGet(cluster, &cluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    s += m_vol->blocksPerCluster();
    // max size if a directory file is 4096 blocks
    if (s >= 4096) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  } while (!m_vol->isEOC(cluster));
  m_fileSize = 512L*s;
  return true;

 fail:
  return false;
}
//------------------------------------------------------------------------------
void FatFile::setpos(FatPos_t* pos) {
  m_curPosition = pos->position;
  m_curCluster = pos->cluster;
}
//------------------------------------------------------------------------------
bool FatFile::sync() {
  // only allow open files and directories
  if (!isOpen()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (m_flags & F_FILE_DIR_DIRTY) {
    dir_t* dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
    // check for deleted by another open file object
    if (!dir || dir->name[0] == DIR_NAME_DELETED) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    // do not set filesize for dir files
    if (!isDir()) dir->fileSize = m_fileSize;

    // update first cluster fields
    dir->firstClusterLow = m_firstCluster & 0XFFFF;
    dir->firstClusterHigh = m_firstCluster >> 16;

    // set modify time if user supplied a callback date/time function
    if (m_dateTime) {
      m_dateTime(&dir->lastWriteDate, &dir->lastWriteTime);
      dir->lastAccessDate = dir->lastWriteDate;
    }
    // clear directory dirty
    m_flags &= ~F_FILE_DIR_DIRTY;
  }
  return m_vol->cacheSync();

 fail:
  m_writeError = true;
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::timestamp(FatFile* file) {
  dir_t* dir;
  dir_t srcDir;

  // get timestamps
  if (!file->dirEntry(&srcDir)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // update directory fields
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // copy timestamps
  dir->lastAccessDate = srcDir.lastAccessDate;
  dir->creationDate = srcDir.creationDate;
  dir->creationTime = srcDir.creationTime;
  dir->creationTimeTenths = srcDir.creationTimeTenths;
  dir->lastWriteDate = srcDir.lastWriteDate;
  dir->lastWriteTime = srcDir.lastWriteTime;

  // write back entry
  return m_vol->cacheSync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::timestamp(uint8_t flags, uint16_t year, uint8_t month,
         uint8_t day, uint8_t hour, uint8_t minute, uint8_t second) {
  uint16_t dirDate;
  uint16_t dirTime;
  dir_t* dir;

  if (!isOpen()
    || year < 1980
    || year > 2107
    || month < 1
    || month > 12
    || day < 1
    || day > 31
    || hour > 23
    || minute > 59
    || second > 59) {
      DBG_FAIL_MACRO;
      goto fail;
  }
  // update directory entry
  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  dir = cacheDirEntry(FatCache::CACHE_FOR_WRITE);
  if (!dir) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  dirDate = FAT_DATE(year, month, day);
  dirTime = FAT_TIME(hour, minute, second);
  if (flags & T_ACCESS) {
    dir->lastAccessDate = dirDate;
  }
  if (flags & T_CREATE) {
    dir->creationDate = dirDate;
    dir->creationTime = dirTime;
    // seems to be units of 1/100 second not 1/10 as Microsoft states
    dir->creationTimeTenths = second & 1 ? 100 : 0;
  }
  if (flags & T_WRITE) {
    dir->lastWriteDate = dirDate;
    dir->lastWriteTime = dirTime;
  }
  return m_vol->cacheSync();

 fail:
  return false;
}
//------------------------------------------------------------------------------
bool FatFile::truncate(uint32_t length) {
  uint32_t newPos;
  // error if not a normal file or read-only
  if (!isFile() || !(m_flags & O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // error if length is greater than current size
  if (length > m_fileSize) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // fileSize and length are zero - nothing to do
  if (m_fileSize == 0) return true;

  // remember position for seek after truncation
  newPos = m_curPosition > length ? length : m_curPosition;

  // position to last cluster in truncated file
  if (!seekSet(length)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  if (length == 0) {
    // free all clusters
    if (!m_vol->freeChain(m_firstCluster)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    m_firstCluster = 0;
  } else {
    uint32_t toFree;
    if (!m_vol->fatGet(m_curCluster, &toFree)) {
      DBG_FAIL_MACRO;
      goto fail;
    }
    if (!m_vol->isEOC(toFree)) {
      // free extra clusters
      if (!m_vol->freeChain(toFree)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      // current cluster is end of chain
      if (!m_vol->fatPutEOC(m_curCluster)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
  }
  m_fileSize = length;

  // need to update directory entry
  m_flags |= F_FILE_DIR_DIRTY;

  if (!sync()) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // set file to correct position
  return seekSet(newPos);

 fail:
  return false;
}
//------------------------------------------------------------------------------
int FatFile::write(const void* buf, size_t nbyte) {
  // convert void* to uint8_t*  -  must be before goto statements
  const uint8_t* src = reinterpret_cast<const uint8_t*>(buf);
  cache_t* pc;
  uint8_t cacheOption;
  // number of bytes left to write  -  must be before goto statements
  size_t nToWrite = nbyte;
  size_t n;
  // error if not a normal file or is read-only
  if (!isFile() || !(m_flags & O_WRITE)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  // seek to end of file if append flag
  if ((m_flags & O_APPEND)) {
    if (!seekEnd()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  // Don't exceed max fileSize.
  if (nbyte > (0XFFFFFFFF - m_curPosition)) {
    DBG_FAIL_MACRO;
    goto fail;
  }
  while (nToWrite) {
    uint8_t blockOfCluster = m_vol->blockOfCluster(m_curPosition);
    uint16_t blockOffset = m_curPosition & 0X1FF;
    if (blockOfCluster == 0 && blockOffset == 0) {
      // start of new cluster
      if (m_curCluster != 0) {
        uint32_t next;
        if (!m_vol->fatGet(m_curCluster, &next)) {
          DBG_FAIL_MACRO;
          goto fail;
        }
        if (m_vol->isEOC(next)) {
          // add cluster if at end of chain
          if (!addCluster()) {
            DBG_FAIL_MACRO;
            goto fail;
          }
        } else {
          m_curCluster = next;
        }
      } else {
        if (m_firstCluster == 0) {
          // allocate first cluster of file
          if (!addCluster()) {
            DBG_FAIL_MACRO;
            goto fail;
          }
          m_firstCluster = m_curCluster;
        } else {
          m_curCluster = m_firstCluster;
        }
      }
    }
    // block for data write
    uint32_t block = m_vol->clusterStartBlock(m_curCluster) + blockOfCluster;

    if (blockOffset != 0 || nToWrite < 512) {
      // partial block - must use cache
      // max space in block
      n = 512 - blockOffset;
      // lesser of space and amount to write
      if (n > nToWrite) n = nToWrite;

      if (blockOffset == 0 && m_curPosition >= m_fileSize) {
        // start of new block don't need to read into cache
        cacheOption = FatCache::CACHE_RESERVE_FOR_WRITE;
      } else {
        // rewrite part of block
        cacheOption = FatCache::CACHE_FOR_WRITE;
      }
      pc = m_vol->cacheFetchData(block, cacheOption);
      if (!pc) {
        DBG_FAIL_MACRO;
        goto fail;
      }
      uint8_t* dst = pc->data + blockOffset;
      memcpy(dst, src, n);
      if (512 == (n + blockOffset)) {
        // Force write if block is full - improves large writes.
        if (!m_vol->cacheSyncData()) {
          DBG_FAIL_MACRO;
          goto fail;
        }
      }
#if USE_MULTI_BLOCK_IO
    } else if (nToWrite >= 1024) {
     // use multiple block write command
      uint8_t maxBlocks = m_vol->blocksPerCluster() - blockOfCluster;
      uint8_t nBlock = nToWrite >> 9;
      if (nBlock > maxBlocks) nBlock = maxBlocks;
      n = 512*nBlock;
      if (m_vol->cacheBlockNumber() <= block
        && block < (m_vol->cacheBlockNumber() + nBlock)) {
        // invalidate cache if block is in cache
        m_vol->cacheInvalidate();
      }
      if (!m_vol->writeBlocks(block, src, nBlock)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
#endif  // USE_MULTI_BLOCK_IO
    } else {
     // use single block write command
      n = 512;
      if (m_vol->cacheBlockNumber() == block) {
        m_vol->cacheInvalidate();
      }
      if (!m_vol->writeBlock(block, src)) {
        DBG_FAIL_MACRO;
        goto fail;
      }
    }
    m_curPosition += n;
    src += n;
    nToWrite -= n;
  }
  if (m_curPosition > m_fileSize) {
    // update fileSize and insure sync will update dir entry
    m_fileSize = m_curPosition;
    m_flags |= F_FILE_DIR_DIRTY;
  } else if (m_dateTime) {
    // insure sync will update modified date and time
    m_flags |= F_FILE_DIR_DIRTY;
  }

  if (m_flags & O_SYNC) {
    if (!sync()) {
      DBG_FAIL_MACRO;
      goto fail;
    }
  }
  return nbyte;

 fail:
  // return for write error
  m_writeError = true;
  return -1;
}
