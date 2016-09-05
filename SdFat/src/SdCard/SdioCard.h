/* Arduino SdCard Library
 * Copyright (C) 2016 by William Greiman
 *
 * This file is part of the Arduino SdSpiCard Library
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
 * along with the Arduino SdSpiCard Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#ifndef SdioCard_h
#define SdioCard_h
#include "SysCall.h"
#include "BlockDriver.h"
/**
 * \class SdioCard
 * \brief Raw SDIO access to SD and SDHC flash memory cards.
 */
class SdioCard : public BaseBlockDriver {
 public:
  /** Initialize the SD card.
   * \return true for success else false.
   */
  bool begin();
  /**
   * Determine the size of an SD flash memory card.
   *
   * \return The number of 512 byte data blocks in the card
   *         or zero if an error occurs.
   */
  uint32_t cardSize();
  /** \return DMA transfer status. */
  bool dmaBusy();
  /** Erase a range of blocks.
   *
   * \param[in] firstBlock The address of the first block in the range.
   * \param[in] lastBlock The address of the last block in the range.
   *
   * \note This function requests the SD card to do a flash erase for a
   * range of blocks.  The data on the card after an erase operation is
   * either 0 or 1, depends on the card vendor.  The card must support
   * single block erase.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */  
  bool erase(uint32_t firstBlock, uint32_t lastBlock);
  /**
   * \return code for the last error. See SdInfo.h for a list of error codes.
   */
  uint8_t errorCode();
  /** \return error data for last error. */
  uint32_t errorData();
  /** \return error line for last error. Tmp function for debug. */
  uint32_t errorLine();
  /** \return the SD clock frequency in kHz. */
  uint32_t kHzSdClk();
  /**
   * Read a 512 byte block from an SD card.
   *
   * \param[in] lba Logical block to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readBlock(uint32_t lba, uint8_t* dst);
  /**
   * Read multiple 512 byte blocks from an SD card.
   *
   * \param[in] lba Logical block to be read.
   * \param[in] nb Number of blocks to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readBlocks(uint32_t lba, uint8_t* dst, size_t nb);
  /**
   * Read a card's CID register. The CID contains card identification
   * information such as Manufacturer ID, Product name, Product serial
   * number and Manufacturing date.
   *
   * \param[out] cid pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCID(void* cid);
  /**
   * Read a card's CSD register. The CSD contains Card-Specific Data that
   * provides information regarding access to the card's contents.
   *
   * \param[out] csd pointer to area for returned data.
   *
   * \return true for success or false for failure.
   */
  bool readCSD(void* csd);
  /** Read OCR register.
   *
   * \param[out] ocr Value of OCR register.
   * \return true for success else false.
   */
  bool readOCR(uint32_t* ocr);
  /** \return success if sync successful. Not for user apps. */
  bool syncBlocks();
  /** Return the card type: SD V1, SD V2 or SDHC
   * \return 0 - SD V1, 1 - SD V2, or 3 - SDHC.
   */
  uint8_t type();
  /**
   * Writes a 512 byte block to an SD card.
   *
   * \param[in] lba Logical block to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeBlock(uint32_t lba, const uint8_t* src);
  /**
   * Write multiple 512 byte blocks to an SD card.
   *
   * \param[in] lba Logical block to be written.
   * \param[in] nb Number of blocks to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeBlocks(uint32_t lba, const uint8_t* src, size_t nb);
};
#endif  // SdioCard_h
