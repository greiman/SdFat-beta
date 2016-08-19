/* Arduino SdFat Library
 * Copyright (C) 2016 by William Greiman
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
 * \brief Define block driver.
 */
#ifndef BlockDriver_h
#define BlockDriver_h
#ifdef ARDUINO
#include "SdSpiCard/SdSpiCard.h"
#else  // ARDUINO
#include "SdSpiCard.h"
#endif  // ARDUINO
#include "FatLib/BaseBlockDriver.h"
//-----------------------------------------------------------------------------
/**
 * \class SdBlockDriver
 * \brief Standard SD block driver.
 */
#if ENABLE_EXTENDED_TRANSFER_CLASS
class SdBlockDriver : public BaseBlockDriver, public SdSpiCard {
#else  // ENABLE_EXTENDED_TRANSFER_CLASS
class SdBlockDriver : public SdSpiCard {
#endif  // ENABLE_EXTENDED_TRANSFER_CLASS
 public:
  /** Initialize the SD card
   *
   * \param[in] spi SPI driver.
   * \param[in] csPin Card chip select pin number.
   * \param[in] spiSettings SPI speed, mode, and bit order.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool begin(SdSpiDriver* spi, uint8_t csPin, SPISettings spiSettings) {
    spi->begin(csPin);
    spi->setSpiSettings(SD_SCK_HZ(250000));
    if (!SdSpiCard::begin(spi)) {
      return false;
    }
    spi->setSpiSettings(spiSettings);
    return true;
  }
  /**
   * Read a 512 byte block from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool readBlock(uint32_t block, uint8_t* dst) {
    return SdSpiCard::readBlock(block, dst);
  }
  /** End multi-block transfer and go to idle state.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */   
  bool syncBlocks() {
    return true;
  }
  /**
   * Writes a 512 byte block to an SD card.
   *
   * \param[in] block Logical block to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool writeBlock(uint32_t block, const uint8_t* src) {
    return SdSpiCard::writeBlock(block, src);
  }
  /**
   * Read multiple 512 byte blocks from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[in] nb Number of blocks to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool readBlocks(uint32_t block, uint8_t* dst, size_t nb) {
    return SdSpiCard::readBlocks(block, dst, nb);
  }
  /**
   * Write multiple 512 byte blocks to an SD card.
   *
   * \param[in] block Logical block to be written.
   * \param[in] nb Number of blocks to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool writeBlocks(uint32_t block, const uint8_t* src, size_t nb) {
    return SdSpiCard::writeBlocks(block, src, nb);
  }
};
//-----------------------------------------------------------------------------
/**
 * \class SdBlockDriverEX
 * \brief Extended SD I/O block driver.
 */
class SdBlockDriverEX : public SdSpiCard, public BaseBlockDriver {
 public:
  /** Initialize the SD card
   *
   * \param[in] spi SPI driver.
   * \param[in] csPin Card chip select pin number.
   * \param[in] spiSettings SPI speed, mode, and bit order.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */   
  bool begin(SdSpiDriver* spi, uint8_t csPin, SPISettings spiSettings) {
    m_curState = IDLE_STATE;
    spi->begin(csPin);
    spi->setSpiSettings(SD_SCK_HZ(250000));
    if (!SdSpiCard::begin(spi)) {
      return false;
    }
    spi->setSpiSettings(spiSettings);
    return true;
  }
  /**
   * Read a 512 byte block from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool readBlock(uint32_t block, uint8_t* dst);
  /** End multi-block transfer and go to idle state.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool syncBlocks();
  /**
   * Writes a 512 byte block to an SD card.
   *
   * \param[in] block Logical block to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool writeBlock(uint32_t block, const uint8_t* src);
  /**
   * Read multiple 512 byte blocks from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[in] nb Number of blocks to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool readBlocks(uint32_t block, uint8_t* dst, size_t nb);
  /**
   * Write multiple 512 byte blocks to an SD card.
   *
   * \param[in] block Logical block to be written.
   * \param[in] nb Number of blocks to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool writeBlocks(uint32_t block, const uint8_t* src, size_t nb);

 private:
  static const uint32_t IDLE_STATE = 0;
  static const uint32_t READ_STATE = 1;
  static const uint32_t WRITE_STATE = 2;
  uint32_t m_curBlock;
  uint8_t m_curState;
};
//-----------------------------------------------------------------------------
/** typedef for BlockDriver */
#if ENABLE_EXTENDED_TRANSFER_CLASS
typedef BaseBlockDriver BlockDriver;
#else  // ENABLE_EXTENDED_TRANSFER_CLASS
typedef SdBlockDriver BlockDriver;
#endif  // ENABLE_EXTENDED_TRANSFER_CLASS
#endif  // BlockDriver_h
