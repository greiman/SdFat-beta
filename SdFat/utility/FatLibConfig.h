/* FatLib Library
 * Copyright (C) 2013 by William Greiman
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
/**
 * \file
 * \brief configuration definitions
 */
#ifndef FatLibConfig_h
#define FatLibConfig_h
/** Use SdFatConfig.h if nonzero */
#define USE_SDFAT_CONFIG 1
#if USE_SDFAT_CONFIG
#include "../SdFatConfig.h"
#else  //  USE_SDFAT_CONFIG
#include <stdint.h>
#ifdef __AVR__
#include <avr/io.h>
#endif  // __AVR__
//------------------------------------------------------------------------------
/**
 * Set USE_SEPARATE_FAT_CACHE non-zero to use a second 512 byte cache
 * for FAT table entries.  Improves performance for large writes that
 * are not a multiple of 512 bytes.
 */
#ifdef __arm__
#define USE_SEPARATE_FAT_CACHE 1
#else  // __arm__
#define USE_SEPARATE_FAT_CACHE 0
#endif  // __arm__
//------------------------------------------------------------------------------
/**
 * Set USE_MULTI_BLOCK_IO non-zero to use multi-block SD read/write.
 *
 * Don't use mult-block read/write on small AVR boards.
 */
#if defined(RAMEND) && RAMEND < 3000
#define USE_MULTI_BLOCK_IO 0
#else
#define USE_MULTI_BLOCK_IO 1
#endif
//------------------------------------------------------------------------------
/**
 * Set DESTRUCTOR_CLOSES_FILE non-zero to close a file in its destructor.
 *
 * Causes use of lots of heap in ARM.
 */
#define DESTRUCTOR_CLOSES_FILE 0
//------------------------------------------------------------------------------
/**
 * Call flush for endl if ENDL_CALLS_FLUSH is non-zero
 *
 * The standard for iostreams is to call flush.  This is very costly for
 * SdFat.  Each call to flush causes 2048 bytes of I/O to the SD.
 *
 * SdFat has a single 512 byte buffer for I/O so it must write the current
 * data block to the SD, read the directory block from the SD, update the
 * directory entry, write the directory block to the SD and read the data
 * block back into the buffer.
 *
 * The SD flash memory controller is not designed for this many rewrites
 * so performance may be reduced by more than a factor of 100.
 *
 * If ENDL_CALLS_FLUSH is zero, you must call flush and/or close to force
 * all data to be written to the SD.
 */
#define ENDL_CALLS_FLUSH 0
//------------------------------------------------------------------------------
/**
 * Allow FAT12 volumes if FAT12_SUPPORT is non-zero.
 * FAT12 has not been well tested.
 */
#define FAT12_SUPPORT 0
#endif  //  USE_SDFAT_CONFIG
#endif  // FatLibConfig_h
