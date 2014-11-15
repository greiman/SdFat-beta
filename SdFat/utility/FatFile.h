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
#ifndef FatFile_h
#define FatFile_h
/**
 * \file
 * \brief FatFile class
 */
#include <ctype.h>
#include <string.h>
#include <stddef.h>
#include <limits.h>
#include "FatLibConfig.h"
#include "FatApiConstants.h"
#include "FatStructs.h"
#include "FatVolume.h"
class FatFileSystem;
//------------------------------------------------------------------------------
#if defined(ARDUINO) || defined(DOXYGEN)
#include <Arduino.h>
/** Use Print on Arduino */
typedef Print print_t;
#else  // ARDUINO
//  Arduino type for flash string.
class __FlashStringHelper;
/**
 * \class CharWriter
 * \brief Character output - often serial port.
 */
class CharWriter {
 public:
  virtual size_t write(char c) = 0;
  virtual size_t write(const char* s) = 0;
};
typedef Print print_t;
#endif  // ARDUINO
//------------------------------------------------------------------------------
// Stuff to store strings in AVR flash.
#ifdef __AVR__
#include <avr/pgmspace.h>
#else  // __AVR__
#ifndef PGM_P
/** pointer to flash for ARM */
#define PGM_P const char*
#endif  // PGM_P
#ifndef PSTR
/** store literal string in flash for ARM */
#define PSTR(x) (x)
#endif  // PSTR
#ifndef pgm_read_byte
/** read 8-bits from flash for ARM */
#define pgm_read_byte(addr) (*(const unsigned char*)(addr))
#endif  // pgm_read_byte
#ifndef pgm_read_word
/** read 16-bits from flash for ARM */
#define pgm_read_word(addr) (*(const uint16_t*)(addr))
#endif  // pgm_read_word
#ifndef PROGMEM
/** store in flash for ARM */
#define PROGMEM const
#endif  // PROGMEM
#endif  // __AVR__
//------------------------------------------------------------------------------
/**
 * \struct FatPos_t
 * \brief Internal type for file position - do not use in user apps.
 */
struct FatPos_t {
  /** stream position */
  uint32_t position;
  /** cluster for position */
  uint32_t cluster;
  FatPos_t() : position(0), cluster(0) {}
};
//==============================================================================
/**
 * \class FatFile
 * \brief Basic file class.
 */
class FatFile {
 public:
#ifndef DOXYGEN_SHOULD_SKIP_THIS
  // development tests - DO NOT USE!
  bool createLfn(FatFile* dirFile,  ////////////////////////////////////////////////////
                 uint16_t bgnIndex, char* name, uint8_t oflag);  ///////////////////////
  bool findLfn(const char* name, uint16_t* bgnIndex, uint16_t *endIndex);  /////////////
  bool findSfn(uint8_t* sfn, uint16_t* index);  ////////////////////////////////////////
#endif  // DOXYGEN_SHOULD_SKIP_THIS
  /** Create an instance. */
  FatFile() : m_writeError(false), m_attr(FILE_ATTR_CLOSED) {}
  /**  Create a file object and open it in the current working directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a bitwise-inclusive
   * OR of open flags. see FatFile::open(FatFile*, const char*, uint8_t).
   */
  FatFile(const char* path, uint8_t oflag) {
    m_attr = FILE_ATTR_CLOSED;
    m_writeError = false;
    open(path, oflag);
  }
#if DESTRUCTOR_CLOSES_FILE
  ~FatFile() {if(isOpen()) close();}
#endif  // DESTRUCTOR_CLOSES_FILE

  /** \return value of writeError */
  bool getWriteError() {return m_writeError;}
  /** Set writeError to zero */
  void clearWriteError() {m_writeError = 0;}
  //----------------------------------------------------------------------------
  // helpers for stream classes
  /** get position for streams
   * \param[out] pos struct to receive position
   */
  void getpos(FatPos_t* pos);
  /** set position for streams
   * \param[out] pos struct with value for new position
   */
  void setpos(FatPos_t* pos);
  //----------------------------------------------------------------------------
  /** \return number of bytes available from the current position to EOF */
  uint32_t available() {return fileSize() - curPosition();}
  /** Close a file and force cached data and directory information
   *  to be written to the storage device.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include no file is open or an I/O error.
   */
  bool close();
  /** Check for contiguous file and return its raw block range.
   *
   * \param[out] bgnBlock the first block address for the file.
   * \param[out] endBlock the last  block address for the file.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include file is not contiguous, file has zero length
   * or an I/O error occurred.
   */
  bool contiguousRange(uint32_t* bgnBlock, uint32_t* endBlock);
  /** Create and open a new contiguous file of a specified size.
   *
   * \note This function only supports short DOS 8.3 names.
   * See open() for more information.
   *
   * \param[in] dirFile The directory where the file will be created.
   * \param[in] path A path with a valid DOS 8.3 file name.
   * \param[in] size The desired file size.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include \a path contains
   * an invalid DOS 8.3 file name, the FAT volume has not been initialized,
   * a file is already open, the file already exists, the root
   * directory is full or an I/O error.
   *
   */
  bool createContiguous(FatFile* dirFile,
          const char* path, uint32_t size);
  /** \return The current cluster number for a file or directory. */
  uint32_t curCluster() const {return m_curCluster;}
  /** \return The current position for a file or directory. */
  uint32_t curPosition() const {return m_curPosition;}
  /** \return Current working directory */
  static FatFile* cwd() {return m_cwd;}
  /** Set the date/time callback function
   *
   * \param[in] dateTime The user's call back function.  The callback
   * function is of the form:
   *
   * \code
   * void dateTime(uint16_t* date, uint16_t* time) {
   *   uint16_t year;
   *   uint8_t month, day, hour, minute, second;
   *
   *   // User gets date and time from GPS or real-time clock here
   *
   *   // return date using FAT_DATE macro to format fields
   *   *date = FAT_DATE(year, month, day);
   *
   *   // return time using FAT_TIME macro to format fields
   *   *time = FAT_TIME(hour, minute, second);
   * }
   * \endcode
   *
   * Sets the function that is called when a file is created or when
   * a file's directory entry is modified by sync(). All timestamps,
   * access, creation, and modify, are set when a file is created.
   * sync() maintains the last access date and last modify date/time.
   *
   * See the timestamp() function.
   */
  static void dateTimeCallback(
    void (*dateTime)(uint16_t* date, uint16_t* time)) {
    m_dateTime = dateTime;
  }
  /**  Cancel the date/time callback function. */
  static void dateTimeCallbackCancel() {m_dateTime = 0;}
  /** Return a file's directory entry.
   *
   * \param[out] dir Location for return of the file's directory entry.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool dirEntry(dir_t* dir);

  /**
   * \return The index of this file in it's directory.
   */
  uint16_t dirIndex() {return m_dirIndex;}
  /** Format the name field of \a dir into the 13 byte array
   * \a name in standard 8.3 short name format.
   *
   * \param[in] dir The directory structure containing the name.
   * \param[out] name A 13 byte char array for the formatted name.
   * \return length of the name.
   */
  static uint8_t dirName(const dir_t* dir, char* name);
  /** Test for the existence of a file in a directory
   *
   * \param[in] path Path of the file to be tested for.
   *
   * The calling instance must be an open directory file.
   *
   * dirFile.exists("TOFIND.TXT") searches for "TOFIND.TXT" in  the directory
   * dirFile.
   *
   * \return true if the file exists else false.
   */
  bool exists(const char* path) {
    FatFile file;
    return file.open(this, path, O_READ);
  }
  /**
   * Get a string from a file.
   *
   * fgets() reads bytes from a file into the array pointed to by \a str, until
   * \a num - 1 bytes are read, or a delimiter is read and transferred to \a str,
   * or end-of-file is encountered. The string is then terminated
   * with a null byte.
   *
   * fgets() deletes CR, '\\r', from the string.  This insures only a '\\n'
   * terminates the string for Windows text files which use CRLF for newline.
   *
   * \param[out] str Pointer to the array where the string is stored.
   * \param[in] num Maximum number of characters to be read
   * (including the final null byte). Usually the length
   * of the array \a str is used.
   * \param[in] delim Optional set of delimiters. The default is "\n".
   *
   * \return For success fgets() returns the length of the string in \a str.
   * If no data is read, fgets() returns zero for EOF or -1 if an error occurred.
   */
  int16_t fgets(char* str, int16_t num, char* delim = 0);
  /** \return The total number of bytes in a file or directory. */
  uint32_t fileSize() const {return m_fileSize;}
  /** \return The first cluster number for a file or directory. */
  uint32_t firstCluster() const {return m_firstCluster;}
  /** Get a file's name
   *
   * \param[out] name An array of 13 characters for the file's name.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool getFilename(char* name);

  /** \return True if this is a directory else false. */
  bool isDir() const {return m_attr & FILE_ATTR_DIR;}
  /** \return True if this is a normal file else false. */
  bool isFile() const {return !isDir();}
  /** \return True if this is a hidden file else false. */
  bool isHidden() const {return m_attr & FILE_ATTR_HIDDEN;}
  /** \return True if this is an open file/directory else false. */
  bool isOpen() const {return m_attr & FILE_ATTR_IS_OPEN;}
  /** \return True if this is the root directory. */
  bool isRoot() const {return m_attr & FILE_ATTR_ROOT;}
  /** \return True if file is read-only */
  bool isReadOnly() const {return m_attr & FILE_ATTR_READ_ONLY;}
  /** \return True if this is the FAT12 of FAT16 root directory. */
  bool isRootFixed() const {return m_attr & FILE_ATTR_ROOT_FIXED;}
  /** \return True if this is a subdirectory else false. */
  bool isSubDir() const {return m_attr & FILE_ATTR_SUBDIR;}
  /** \return True if this is a system file else false. */
  bool isSystem() const {return m_attr & FILE_ATTR_SYSTEM;}

  /** List directory contents.
   *
   * \param[in] pr Print stream for list.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   *
   * \param[in] indent Amount of space before file name. Used for recursive
   * list to indicate subdirectory level.
   */
  void ls(print_t* pr, uint8_t flags = 0, uint8_t indent = 0);
  /** Make a new directory.
   *
   * \param[in] dir An open FatFile instance for the directory that will
   *                   contain the new directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for the new directory.
   *
   * \param[in] pFlag Create missing parent directories if true.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include this file is already open, \a parent is not a
   * directory, \a path is invalid or already exists in \a parent.
   */
  bool mkdir(FatFile* dir, const char* path, bool pFlag = true);
  /** Open a file in the volume working directory of a FatFileSystem.
   *
   * \param[in] fs File System where the file is located.
   *
   * \param[in] path with a valid 8.3 DOS name for a file to be opened.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see FatFile::open(FatFile*, const char*, uint8_t).
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool open(FatFileSystem* fs, const char* path, uint8_t oflag);
  /** Open a file by index.
   *
   * \param[in] dirFile An open FatFile instance for the directory.
   *
   * \param[in] index The \a index of the directory entry for the file to be
   * opened.  The value for \a index is (directory file position)/32.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see FatFile::open(FatFile*, const char*, uint8_t).
   *
   * See open() by path for definition of flags.
   * \return true for success or false for failure.
   */
  bool open(FatFile* dirFile, uint16_t index, uint8_t oflag);
  /** Open a file or directory by name.
   *
   * \param[in] dirFile An open FatFile instance for the directory containing
   *                    the file to be opened.
   *
   * \param[in] path A path with a valid 8.3 DOS name for a file to be opened.
   *
   * \param[in] oflag Values for \a oflag are constructed by a
   *                  bitwise-inclusive OR of flags from the following list
   *
   * O_READ - Open for reading.
   *
   * O_RDONLY - Same as O_READ.
   *
   * O_WRITE - Open for writing.
   *
   * O_WRONLY - Same as O_WRITE.
   *
   * O_RDWR - Open for reading and writing.
   *
   * O_APPEND - If set, the file offset shall be set to the end of the
   * file prior to each write.
   *
   * O_AT_END - Set the initial position at the end of the file.
   *
   * O_CREAT - If the file exists, this flag has no effect except as noted
   * under O_EXCL below. Otherwise, the file shall be created
   *
   * O_EXCL - If O_CREAT and O_EXCL are set, open() shall fail if the file exists.
   *
   * O_SYNC - Call sync() after each write.  This flag should not be used with
   * write(uint8_t) or any functions do character at a time writes since sync()
   * will be called after each byte.
   *
   * O_TRUNC - If the file exists and is a regular file, and the file is
   * successfully opened and is not read only, its length shall be truncated to 0.
   *
   * WARNING: A given file must not be opened by more than one FatFile object
   * or file corruption may occur.
   *
   * \note Directory files must be opened read only.  Write and truncation is
   * not allowed for directory files.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include this file is already open, \a dirFile is not
   * a directory, \a path is invalid, the file does not exist
   * or can't be opened in the access mode specified by oflag.
   */
  bool open(FatFile* dirFile, const char* path, uint8_t oflag);
  /** Open a file in the current working directory.
   *
   * \param[in] path A path with a valid 8.3 DOS name for a file to be opened.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see FatFile::open(FatFile*, const char*, uint8_t).
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool open(const char* path, uint8_t oflag = O_READ) {
    return open(m_cwd, path, oflag);
  }
  /** Open the next file or subdirectory in a directory.
   *
   * \param[in] dirFile An open FatFile instance for the directory
   *                    containing the file to be opened.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see FatFile::open(FatFile*, const char*, uint8_t).
   *
   * \return true for success or false for failure.
   */
  bool openNext(FatFile* dirFile, uint8_t oflag = O_READ);
  /** Open the next file or subdirectory in a directory and return the
   *  file name.  The Long %File Name, LFN, is returned if present otherwise
   *  the 8.3 short name will be returned.
   *
   * \param[in] dirFile An open FatFile instance for the directory
   *                    containing the file to be opened.
   *
   * \param[out] name Location that will receive the name.
   *
   * \param[in] size The size of the name array.
   *
   * \param[in] oflag bitwise-inclusive OR of open mode flags.
   *                  See see FatFile::open(FatFile*, const char*, uint8_t).
   *
   * \return For success, the length of the returned name.  The name
   *         will be truncated to size - 1 bytes followed by a zero byte.
   *         For EOF, zero will be returned. If an error occurs, -1 is
   *         returned.
   */
  int openNextLFN(FatFile* dirFile, char* name, size_t size, uint8_t oflag);
  /** Open a volume's root directory.
   *
   * \param[in] vol The FAT volume containing the root directory to be opened.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include the file is already open, the FAT volume has
   * not been initialized or it a FAT12 volume.
   */
  bool openRoot(FatVolume* vol);
  /** Return the next available byte without consuming it.
   *
   * \return The byte if no error and not at eof else -1;
   */
  int peek();
  /** Print a file's creation date and time
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool printCreateDateTime(print_t* pr);
  /** %Print a directory date field.
   *
   *  Format is yyyy-mm-dd.
   *
   * \param[in] pr Print stream for output.
   * \param[in] fatDate The date field from a directory entry.
   */
  static void printFatDate(print_t* pr, uint16_t fatDate);
  /** %Print a directory time field.
   *
   * Format is hh:mm:ss.
   *
   * \param[in] pr Print stream for output.
   * \param[in] fatTime The time field from a directory entry.
   */
  static void printFatTime(print_t* pr, uint16_t fatTime);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \param[in] prec Number of digits after decimal point.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(float value, char term, uint8_t prec = 2);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(int16_t value, char term);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(uint16_t value, char term);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(int32_t value, char term);
  /** Print a number followed by a field terminator.
   * \param[in] value The number to be printed.
   * \param[in] term The field terminator.  Use '\\n' for CR LF.
   * \return The number of bytes written or -1 if an error occurs.
   */
  int printField(uint32_t value, char term);
  /** Print a file's modify date and time
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool printModifyDateTime(print_t* pr);
  /** Print a file's name
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  size_t printName(print_t* pr);
  /** Print a file's size.
   *
   * \param[in] pr Print stream for output.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  size_t printFileSize(print_t* pr);
  /** Read the next byte from a file.
   *
   * \return For success read returns the next byte in the file as an int.
   * If an error occurs or end of file is reached -1 is returned.
   */
  int read() {
    uint8_t b;
    return read(&b, 1) == 1 ? b : -1;
  }
  /** Read data from a file starting at the current position.
   *
   * \param[out] buf Pointer to the location that will receive the data.
   *
   * \param[in] nbyte Maximum number of bytes to read.
   *
   * \return For success read() returns the number of bytes read.
   * A value less than \a nbyte, including zero, will be returned
   * if end of file is reached.
   * If an error occurs, read() returns -1.  Possible errors include
   * read() called before a file has been opened, corrupt file system
   * or an I/O error occurred.
   */
  int read(void* buf, size_t nbyte);
  /** Read the next directory entry from a directory file.
   *
   * \param[out] dir The dir_t struct that will receive the data.
   *
   * \return For success readDir() returns the number of bytes read.
   * A value of zero will be returned if end of file is reached.
   * If an error occurs, readDir() returns -1.  Possible errors include
   * readDir() called before a directory has been opened, this is not
   * a directory file or an I/O error occurred.
   */
  int8_t readDir(dir_t* dir);
  /** Remove a file.
   *
   * The directory entry and all data for the file are deleted.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * file that has a long name. For example if a file has the long name
   * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include the file read-only, is a directory,
   * or an I/O error occurred.
   */
  bool remove();
  /** Remove a file.
   *
   * The directory entry and all data for the file are deleted.
   *
   * \param[in] dirFile The directory that contains the file.
   * \param[in] path Path for the file to be removed.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * file that has a long name. For example if a file has the long name
   * "New Text Document.txt" you should not delete the 8.3 name "NEWTEX~1.TXT".
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include the file is a directory, is read only,
   * \a dirFile is not a directory, \a path is not found
   * or an I/O error occurred.
   */
  static bool remove(FatFile* dirFile, const char* path);
  /** Set the file's current position to zero. */
  void rewind() {seekSet(0);}
  /** Rename a file or subdirectory.
   *
   * \param[in] dirFile Directory for the new path.
   * \param[in] newPath New path name for the file/directory.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include \a dirFile is not open or is not a directory
   * file, newPath is invalid or already exists, or an I/O error occurs.
   */
  bool rename(FatFile* dirFile, const char* newPath);
  /** Remove a directory file.
   *
   * The directory file will be removed only if it is empty and is not the
   * root directory.  rmdir() follows DOS and Windows and ignores the
   * read-only attribute for the directory.
   *
   * \note This function should not be used to delete the 8.3 version of a
   * directory that has a long name. For example if a directory has the
   * long name "New folder" you should not delete the 8.3 name "NEWFOL~1".
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include the file is not a directory, is the root
   * directory, is not empty, or an I/O error occurred.
   */
  bool rmdir();
  /** Recursively delete a directory and all contained files.
   *
   * This is like the Unix/Linux 'rm -rf *' if called with the root directory
   * hence the name.
   *
   * Warning - This will remove all contents of the directory including
   * subdirectories.  The directory will then be removed if it is not root.
   * The read-only attribute for files will be ignored.
   *
   * \note This function should not be used to delete the 8.3 version of
   * a directory that has a long name.  See remove() and rmdir().
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool rmRfStar();
  /** Set the files position to current position + \a pos. See seekSet().
   * \param[in] offset The new position in bytes from the current position.
   * \return true for success or false for failure.
   */
  bool seekCur(int32_t offset) {
    return seekSet(m_curPosition + offset);
  }
  /** Set the files position to end-of-file + \a offset. See seekSet().
   * \param[in] offset The new position in bytes from end-of-file.
   * \return true for success or false for failure.
   */
  bool seekEnd(int32_t offset = 0) {return seekSet(m_fileSize + offset);}
  /** Sets a file's position.
   *
   * \param[in] pos The new position in bytes from the beginning of the file.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool seekSet(uint32_t pos);
  /** Set the current working directory.
   *
   * \param[in] dir New current working directory.
   *
   * \return true for success else false.
   */
  static bool setCwd(FatFile* dir) {
    if (!dir->isDir()) return false;
    m_cwd = dir;
    return true;
  }
  /** The sync() call causes all modified data and directory fields
   * to be written to the storage device.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include a call to sync() before a file has been
   * opened or an I/O error.
   */
  bool sync();
  /** Copy a file's timestamps
   *
   * \param[in] file File to copy timestamps from.
   *
   * \note
   * Modify and access timestamps may be overwritten if a date time callback
   * function has been set by dateTimeCallback().
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool timestamp(FatFile* file);
  /** Set a file's timestamps in its directory entry.
   *
   * \param[in] flags Values for \a flags are constructed by a bitwise-inclusive
   * OR of flags from the following list
   *
   * T_ACCESS - Set the file's last access date.
   *
   * T_CREATE - Set the file's creation date and time.
   *
   * T_WRITE - Set the file's last write/modification date and time.
   *
   * \param[in] year Valid range 1980 - 2107 inclusive.
   *
   * \param[in] month Valid range 1 - 12 inclusive.
   *
   * \param[in] day Valid range 1 - 31 inclusive.
   *
   * \param[in] hour Valid range 0 - 23 inclusive.
   *
   * \param[in] minute Valid range 0 - 59 inclusive.
   *
   * \param[in] second Valid range 0 - 59 inclusive
   *
   * \note It is possible to set an invalid date since there is no check for
   * the number of days in a month.
   *
   * \note
   * Modify and access timestamps may be overwritten if a date time callback
   * function has been set by dateTimeCallback().
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   */
  bool timestamp(uint8_t flags, uint16_t year, uint8_t month, uint8_t day,
          uint8_t hour, uint8_t minute, uint8_t second);
  /** Type of file.  You should use isFile() or isDir() instead of fileType()
   * if possible.
   *
   * \return The file or directory type.
   */
  uint8_t fileAttr() const {return m_attr;}
  /** Truncate a file to a specified length.  The current file position
   * will be maintained if it is less than or equal to \a length otherwise
   * it will be set to end of file.
   *
   * \param[in] length The desired length for the file.
   *
   * \return The value one, true, is returned for success and
   * the value zero, false, is returned for failure.
   * Reasons for failure include file is read only, file is a directory,
   * \a length is greater than the current file size or an I/O error occurs.
   */
  bool truncate(uint32_t length);
  /** \return FatVolume that contains this file. */
  FatVolume* volume() const {return m_vol;}
  /** Write a single byte.
   * \param[in] b The byte to be written.
   * \return +1 for success or -1 for failure.
   */
  int write(uint8_t b) {return write(&b, 1);}
  /** Write data to an open file.
   *
   * \note Data is moved to the cache but may not be written to the
   * storage device until sync() is called.
   *
   * \param[in] buf Pointer to the location of the data to be written.
   *
   * \param[in] nbyte Number of bytes to write.
   *
   * \return For success write() returns the number of bytes written, always
   * \a nbyte.  If an error occurs, write() returns -1.  Possible errors
   * include write() is called before a file has been opened, write is called
   * for a read-only file, device is full, a corrupt file system or an I/O error.
   *
   */
  int write(const void* buf, size_t nbyte);
//------------------------------------------------------------------------------
 private:
  /** This file has not been opened. */
  static const uint8_t FILE_ATTR_CLOSED = 0;
  /** File is read-only. */
  static const uint8_t FILE_ATTR_READ_ONLY = DIR_ATT_READ_ONLY;
  /** File should be hidden in directory listings. */
  static const uint8_t FILE_ATTR_HIDDEN = DIR_ATT_HIDDEN;
  /** Entry is for a system file. */
  static const uint8_t FILE_ATTR_SYSTEM = DIR_ATT_SYSTEM;
  /** Entry for normal data file */
  static const uint8_t FILE_ATTR_IS_OPEN = 0X08;
  /** Entry is for a subdirectory */
  static const uint8_t FILE_ATTR_SUBDIR = DIR_ATT_DIRECTORY;
  /** A FAT12 or FAT16 root directory */
  static const uint8_t FILE_ATTR_ROOT_FIXED = 0X20;
  /** A FAT32 root directory */
  static const uint8_t FILE_ATTR_ROOT32 = 0X40;
  /** Entry is for root. */
  static const uint8_t FILE_ATTR_ROOT = FILE_ATTR_ROOT_FIXED | FILE_ATTR_ROOT32;
  /** Directory type bits */
  static const uint8_t FILE_ATTR_DIR = FILE_ATTR_SUBDIR | FILE_ATTR_ROOT;
  /** Attributes to copy from directory entry */
  static const uint8_t FILE_ATTR_COPY = DIR_ATT_READ_ONLY | DIR_ATT_HIDDEN |
                                        DIR_ATT_SYSTEM | DIR_ATT_DIRECTORY;

  /** experimental don't use */
  bool openParent(FatFile* dir);

  // private functions
  bool addCluster();
  bool addDirCluster();
  dir_t* cacheDirEntry(uint8_t action);
  int8_t lsPrintNext(print_t* pr, uint8_t flags, uint8_t indent);
  static bool make83Name(const char* str, uint8_t* name, const char** ptr);
  bool mkdir(FatFile* parent, const uint8_t dname[11]);
  bool open(FatFile* dirFile, const uint8_t dname[11], uint8_t oflag);
  bool openCachedEntry(FatFile* dirFile, uint16_t cacheIndex, uint8_t oflag);
  bool readLBN(uint32_t* lbn);
  dir_t* readDirCache();
  bool setDirSize();

  // bits defined in m_flags
  // should be 0X0F
  static uint8_t const F_OFLAG = (O_ACCMODE | O_APPEND | O_SYNC);
  // sync of directory entry required
  static uint8_t const F_FILE_DIR_DIRTY = 0X80;

  // global pointer to cwd dir
  static FatFile* m_cwd;
  // data time callback function
  static void (*m_dateTime)(uint16_t* date, uint16_t* time);
  // private data
  bool       m_writeError;       // Set when a write error occurs
  uint8_t    m_attr;             // File attributes
  uint8_t    m_flags;            // See above for definition of m_flags bits
  uint16_t   m_dirIndex;         // index of directory entry in dir file
  FatVolume* m_vol;              // volume where file is located
  uint32_t   m_curCluster;       // cluster for current file position
  uint32_t   m_curPosition;      // current file position
  uint32_t   m_dirBlock;         // block for this files directory entry
  uint32_t   m_dirFirstCluster;  // first cluster of this file's directory
  uint32_t   m_fileSize;         // file size in bytes
  uint32_t   m_firstCluster;     // first cluster of file
};
#endif  // FatFile_h
