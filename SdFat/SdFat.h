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
#ifndef SdFat_h
#define SdFat_h
/**
 * \file
 * \brief SdFat class
 */
#include "SdSpiCard.h"
#include "SdFile.h"
#include "utility/FatLib.h"
//------------------------------------------------------------------------------
/** SdFat version YYYYMMDD */
#define SD_FAT_VERSION 20141204
//==============================================================================
/**
 * \class SdFatBase
 * \brief Virtual base class for %SdFat library.
 */
class SdFatBase : public FatFileSystem {
 public:
  /** Initialize SD card and file system.
   * \param[in] spi SPI object for the card.
   * \param[in] csPin SD card chip select pin.
   * \param[in] divisor SPI divisor.
   * \return true for success else false.
   */
  bool begin(SdSpiCard::m_spi_t* spi, uint8_t csPin = SS, uint8_t divisor = 2) {
    return m_sdCard.begin(spi, csPin, divisor) &&
           FatFileSystem::begin(&m_vwd);
  }
  /** \return Pointer to SD card object */
  SdSpiCard *card() {
    return &m_sdCard;
  }
  /** %Print any SD error code to Serial and halt. */
  void errorHalt() {
    errorHalt(&Serial);
  }
  /** %Print any SD error code and halt.
   *
   * \param[in] pr Print destination.
   */
  void errorHalt(Print* pr);
  /** %Print msg, any SD error code and halt.
   *
   * \param[in] msg Message to print.
   */
  void errorHalt(char const* msg) {
    errorHalt(&Serial, msg);
  }
  /** %Print msg, any SD error code, and halt.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void errorHalt(Print* pr, char const* msg);
  /** %Print msg, any SD error code, and halt.
   *
   * \param[in] msg Message to print.
   */
  void errorHalt(const __FlashStringHelper* msg) {
    errorHalt(&Serial, msg);
  }
  /** %Print msg, any SD error code, and halt.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void errorHalt(Print* pr, const __FlashStringHelper* msg);
  /** %Print any SD error code to Serial */
  void errorPrint() {
    errorPrint(&Serial);
  }
  /** %Print any SD error code.
   * \param[in] pr Print device.
   */
  void errorPrint(Print* pr);
  /** %Print msg, any SD error code.
   *
   * \param[in] msg Message to print.
   */
  void errorPrint(const char* msg) {
    errorPrint(&Serial, msg);
  }
  /** %Print msg, any SD error code.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void errorPrint(Print* pr, char const* msg);
  /** %Print msg, any SD error code.
   *
   * \param[in] msg Message to print.
   */
  void errorPrint(const __FlashStringHelper* msg) {
    errorPrint(&Serial, msg);
  }
  /** %Print msg, any SD error code.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void errorPrint(Print* pr, const __FlashStringHelper* msg);
  /** Diagnostic call to initialize FatFileSystem - use for
   *  diagnostic purposes only.
   *  \return true for success else false.
   */
  bool fsBegin() {
    return FatFileSystem::begin(&m_vwd);
  }
  /** %Print any SD error code and halt. */
  void initErrorHalt() {
    initErrorHalt(&Serial);
  }
  /** %Print error details and halt after begin fails.
   *
   * \param[in] pr Print destination.
   */
  void initErrorHalt(Print* pr);
  /**Print message, error details, and halt after SdFat::init() fails.
   *
   * \param[in] msg Message to print.
   */
  void initErrorHalt(char const *msg) {
    initErrorHalt(&Serial, msg);
  }
  /**Print message, error details, and halt after SdFatBase::init() fails.
   * \param[in] pr Print device.
   * \param[in] msg Message to print.
   */
  void initErrorHalt(Print* pr, char const *msg);
  /**Print message, error details, and halt after SdFat::init() fails.
    *
    * \param[in] msg Message to print.
    */
  void initErrorHalt(const __FlashStringHelper* msg) {
    initErrorHalt(&Serial, msg);
  }
  /**Print message, error details, and halt after SdFatBase::init() fails.
   * \param[in] pr Print device for message.
   * \param[in] msg Message to print.
   */
  void initErrorHalt(Print* pr, const __FlashStringHelper* msg);
  /** Print error details after SdFat::init() fails. */
  void initErrorPrint() {
    initErrorPrint(&Serial);
  }
  /** Print error details after SdFatBase::init() fails.
   *
   * \param[in] pr Print destination.
   */
  void initErrorPrint(Print* pr);
  /**Print message and error details and halt after SdFat::init() fails.
   *
   * \param[in] msg Message to print.
   */
  void initErrorPrint(char const *msg) {
    initErrorPrint(&Serial, msg);
  }
  /**Print message and error details and halt after SdFatBase::init() fails.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void initErrorPrint(Print* pr, char const *msg);
  /**Print message and error details and halt after SdFat::init() fails.
   *
   * \param[in] msg Message to print.
   */
  void initErrorPrint(const __FlashStringHelper* msg) {
    initErrorPrint(&Serial, msg);
  }
  /**Print message and error details and halt after SdFatBase::init() fails.
   *
   * \param[in] pr Print destination.
   * \param[in] msg Message to print.
   */
  void initErrorPrint(Print* pr, const __FlashStringHelper* msg);
  /** List the directory contents of the volume working directory to Serial.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   */
  void ls(uint8_t flags = 0) {
    ls(&Serial, flags);
  }
  /** List the directory contents of a directory to Serial.
   *
   * \param[in] path directory to list.
   *
   * \param[in] flags The inclusive OR of
   *
   * LS_DATE - %Print file modification date
   *
   * LS_SIZE - %Print file size.
   *
   * LS_R - Recursive list of subdirectories.
   */
  void ls(const char* path, uint8_t flags = 0) {
    ls(&Serial, path, flags);
  }
  /** open a file
   *
   * \param[in] path location of file to be opened.
   * \param[in] mode open mode flags.
   * \return a File object.
   */
  File open(const char *path, uint8_t mode = FILE_READ);
  /** \return a pointer to the volume working directory. */
  SdBaseFile* vwd() {
    return &m_vwd;
  }

  using FatFileSystem::ls;

 private:
  uint8_t cardErrorCode() {
    return m_sdCard.errorCode();
  }
  uint8_t cardErrorData() {
    return m_sdCard.errorData();
  }
  bool readBlock(uint32_t block, uint8_t* dst) {
    return m_sdCard.readBlock(block, dst);
  }
  bool writeBlock(uint32_t block, const uint8_t* src) {
    return m_sdCard.writeBlock(block, src);
  }
  bool readBlocks(uint32_t block, uint8_t* dst, size_t n) {
    return m_sdCard.readBlocks(block, dst, n);
  }
  bool writeBlocks(uint32_t block, const uint8_t* src, size_t n) {
    return m_sdCard.writeBlocks(block, src, n);
  }
  SdBaseFile m_vwd;
  SdSpiCard m_sdCard;
};
//==============================================================================
/**
 * \class SdFat
 * \brief Main file system class for %SdFat library.
 */
class SdFat : public SdFatBase {
 public:
  /** Initialize SD card and file system.
   *
   * \param[in] csPin SD card chip select pin.
   * \param[in] divisor SPI divisor.
   * \return true for success else false.
   */
  bool begin(uint8_t csPin = SS, uint8_t divisor = 2) {
    return SdFatBase::begin(&m_spi, csPin, divisor);
  }
  /** Diagnostic call to initialize SD card - use for diagnostic purposes only.
   * \param[in] csPin SD card chip select pin.
   * \param[in] divisor SPI divisor.
   * \return true for success else false.
   */
  bool cardBegin(uint8_t csPin = SS, uint8_t divisor = 2) {
    return card()->begin(&m_spi, csPin, divisor);
  }
 private:
  SpiDefault_t m_spi;
};
//==============================================================================
#if SD_SPI_CONFIGURATION >= 3 || defined(DOXYGEN)
/**
 * \class SdFatLibSpi
 * \brief SdFat class using the standard Arduino SPI library.
 */
class SdFatLibSpi: public SdFatBase {
 public:
  /** Initialize SD card and file system.
  *
  * \param[in] csPin SD card chip select pin.
  * \param[in] divisor SPI divisor.
  * \return true for success else false.
  */
  bool begin(uint8_t csPin = SS, uint8_t divisor = 2) {
    return SdFatBase::begin(&m_spi, csPin, divisor);
  }
  /** Diagnostic call to initialize SD card - use for diagnostic purposes only.
   * \param[in] csPin SD card chip select pin.
   * \param[in] divisor SPI divisor.
   * \return true for success else false.
   */
  bool cardBegin(uint8_t csPin = SS, uint8_t divisor = 2) {
    return card()->begin(&m_spi, csPin, divisor);
  }

 private:
  SdSpiLib m_spi;
};
//==============================================================================
/**
 * \class SdFatSoftSpi
 * \brief SdFat class using software SPI.
 */
template<uint8_t MisoPin, uint8_t MosiPin, uint8_t SckPin>
class SdFatSoftSpi : public SdFatBase {
 public:
  /** Initialize SD card and file system.
   *
   * \param[in] csPin SD card chip select pin.
   * \param[in] divisor SPI divisor.
   * \return true for success else false.
   */
  bool begin(uint8_t csPin = SS, uint8_t divisor = 2) {
    return SdFatBase::begin(&m_spi, csPin, divisor);
  }
  /** Diagnostic call to initialize SD card - use for diagnostic purposes only.
   * \param[in] csPin SD card chip select pin.
   * \param[in] divisor SPI divisor.
   * \return true for success else false.
   */
  bool cardBegin(uint8_t csPin = SS, uint8_t divisor = 2) {
    return card()->begin(&m_spi, csPin, divisor);
  }

 private:
  SdSpiSoft<MisoPin, MosiPin, SckPin> m_spi;
};
#endif  /// SD_SPI_CONFIGURATION >= 3 || defined(DOXYGEN)
#endif  // SdFat_h
