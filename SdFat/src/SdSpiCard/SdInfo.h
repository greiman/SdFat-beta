/* Arduino SdSpiCard Library
 * Copyright (C) 2012 by William Greiman
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
#ifndef SdInfo_h
#define SdInfo_h
#include <stdint.h>
// Based on the document:
//
// SD Specifications
// Part 1
// Physical Layer
// Simplified Specification
// Version 3.01
// May 18, 2010
//
// http://www.sdcard.org/developers/tech/sdcard/pls/simplified_specs
//------------------------------------------------------------------------------
// SD card errors
/** timeout error for command CMD0 (initialize card in SPI mode) */
const uint8_t SD_CARD_ERROR_CMD0 = 0X1;
/** CMD8 was not accepted - not a valid SD card*/
const uint8_t SD_CARD_ERROR_CMD8 = 0X2;
/** card returned an error response for CMD12 (stop multiblock read) */
const uint8_t SD_CARD_ERROR_CMD12 = 0X3;
/** card returned an error response for CMD17 (read block) */
const uint8_t SD_CARD_ERROR_CMD17 = 0X4;
/** card returned an error response for CMD18 (read multiple block) */
const uint8_t SD_CARD_ERROR_CMD18 = 0X5;
/** card returned an error response for CMD24 (write block) */
const uint8_t SD_CARD_ERROR_CMD24 = 0X6;
/**  WRITE_MULTIPLE_BLOCKS command failed */
const uint8_t SD_CARD_ERROR_CMD25 = 0X7;
/** card returned an error response for CMD58 (read OCR) */
const uint8_t SD_CARD_ERROR_CMD58 = 0X8;
/** SET_WR_BLK_ERASE_COUNT failed */
const uint8_t SD_CARD_ERROR_ACMD23 = 0X9;
/** ACMD41 initialization process timeout */
const uint8_t SD_CARD_ERROR_ACMD41 = 0XA;
/** card returned a bad CSR version field */
const uint8_t SD_CARD_ERROR_BAD_CSD = 0XB;
/** erase block group command failed */
const uint8_t SD_CARD_ERROR_ERASE = 0XC;
/** card not capable of single block erase */
const uint8_t SD_CARD_ERROR_ERASE_SINGLE_BLOCK = 0XD;
/** Erase sequence timed out */
const uint8_t SD_CARD_ERROR_ERASE_TIMEOUT = 0XE;
/** card returned an error token instead of read data */
const uint8_t SD_CARD_ERROR_READ = 0XF;
/** read CID or CSD failed */
const uint8_t SD_CARD_ERROR_READ_REG = 0X10;
/** timeout while waiting for start of read data */
const uint8_t SD_CARD_ERROR_READ_TIMEOUT = 0X11;
/** card did not accept STOP_TRAN_TOKEN */
const uint8_t SD_CARD_ERROR_STOP_TRAN = 0X12;
/** card returned an error token as a response to a write operation */
const uint8_t SD_CARD_ERROR_WRITE = 0X13;
/** card busy for command */
const uint8_t SD_CARD_ERROR_CMD_BUSY = 0X14;
/** card did not go ready for a multiple block write */
const uint8_t SD_CARD_ERROR_WRITE_MULTIPLE = 0X15;  // Not used
/** card returned an error to a CMD13 status check after a write */
const uint8_t SD_CARD_ERROR_WRITE_PROGRAMMING = 0X16;
/** timeout occurred during write programming */
const uint8_t SD_CARD_ERROR_WRITE_TIMEOUT = 0X17;
/** incorrect rate selected */
const uint8_t SD_CARD_ERROR_SCK_RATE = 0X18;
/** init() not called */
const uint8_t SD_CARD_ERROR_INIT_NOT_CALLED = 0X19;
/** card returned an error for CMD59 (CRC_ON_OFF) */
const uint8_t SD_CARD_ERROR_CMD59 = 0X1A;
/** invalid read CRC */
const uint8_t SD_CARD_ERROR_READ_CRC = 0X1B;
/** SPI DMA error */
const uint8_t SD_CARD_ERROR_SPI_DMA = 0X1C;
/** ACMD13 not accepted */
const uint8_t SD_CARD_ERROR_ACMD13 = 0X1D;
//------------------------------------------------------------------------------
// card types
/** Standard capacity V1 SD card */
const uint8_t SD_CARD_TYPE_SD1  = 1;
/** Standard capacity V2 SD card */
const uint8_t SD_CARD_TYPE_SD2  = 2;
/** High Capacity SD card */
const uint8_t SD_CARD_TYPE_SDHC = 3;
//------------------------------------------------------------------------------
#define SD_SCK_HZ(maxSpeed) SPISettings(maxSpeed, MSBFIRST, SPI_MODE0)
#define SD_SCK_MHZ(maxMhz) SPISettings(1000000UL*maxMhz, MSBFIRST, SPI_MODE0)
// SPI divisor constants
/** Set SCK to max rate of F_CPU/2. */
#define SPI_FULL_SPEED SD_SCK_MHZ(50)
/** Set SCK rate to F_CPU/3 for Due */
#define SPI_DIV3_SPEED SD_SCK_HZ(F_CPU/3)
/** Set SCK rate to F_CPU/4. */
#define SPI_HALF_SPEED SD_SCK_HZ(F_CPU/4)
/** Set SCK rate to F_CPU/6 for Due */
#define SPI_DIV6_SPEED SD_SCK_HZ(F_CPU/6)
/** Set SCK rate to F_CPU/8. */
#define SPI_QUARTER_SPEED SD_SCK_HZ(F_CPU/8)
/** Set SCK rate to F_CPU/16. */
#define SPI_EIGHTH_SPEED SD_SCK_HZ(F_CPU/16)
/** Set SCK rate to F_CPU/32. */
#define SPI_SIXTEENTH_SPEED SD_SCK_HZ(F_CPU/32)
//------------------------------------------------------------------------------
// SD operation timeouts
/** init timeout ms */
const uint16_t SD_INIT_TIMEOUT = 2000;
/** erase timeout ms */
const uint16_t SD_ERASE_TIMEOUT = 10000;
/** read timeout ms */
const uint16_t SD_READ_TIMEOUT = 300;
/** write time out ms */
const uint16_t SD_WRITE_TIMEOUT = 600;
//------------------------------------------------------------------------------
// SD card commands
/** GO_IDLE_STATE - init card in spi mode if CS low */
const uint8_t CMD0 = 0X00;
/** SWITCH_FUNC - Switch Function Command */
const uint8_t CMD6 = 0X06;
/** SEND_IF_COND - verify SD Memory Card interface operating condition.*/
const uint8_t CMD8 = 0X08;
/** SEND_CSD - read the Card Specific Data (CSD register) */
const uint8_t CMD9 = 0X09;
/** SEND_CID - read the card identification information (CID register) */
const uint8_t CMD10 = 0X0A;
/** STOP_TRANSMISSION - end multiple block read sequence */
const uint8_t CMD12 = 0X0C;
/** SEND_STATUS - read the card status register */
const uint8_t CMD13 = 0X0D;
/** READ_SINGLE_BLOCK - read a single data block from the card */
const uint8_t CMD17 = 0X11;
/** READ_MULTIPLE_BLOCK - read a multiple data blocks from the card */
const uint8_t CMD18 = 0X12;
/** WRITE_BLOCK - write a single data block to the card */
const uint8_t CMD24 = 0X18;
/** WRITE_MULTIPLE_BLOCK - write blocks of data until a STOP_TRANSMISSION */
const uint8_t CMD25 = 0X19;
/** ERASE_WR_BLK_START - sets the address of the first block to be erased */
const uint8_t CMD32 = 0X20;
/** ERASE_WR_BLK_END - sets the address of the last block of the continuous
    range to be erased*/
const uint8_t CMD33 = 0X21;
/** ERASE - erase all previously selected blocks */
const uint8_t CMD38 = 0X26;
/** APP_CMD - escape for application specific command */
const uint8_t CMD55 = 0X37;
/** READ_OCR - read the OCR register of a card */
const uint8_t CMD58 = 0X3A;
/** CRC_ON_OFF - enable or disable CRC checking */
const uint8_t CMD59 = 0X3B;
/** SD_STATUS - Send the SD Status. */
const uint8_t ACMD13 = 0X0D;
/** SET_WR_BLK_ERASE_COUNT - Set the number of write blocks to be
     pre-erased before writing */
const uint8_t ACMD23 = 0X17;
/** SD_SEND_OP_COMD - Sends host capacity support information and
    activates the card's initialization process */
const uint8_t ACMD41 = 0X29;
//==============================================================================
/** status for card in the ready state */
const uint8_t R1_READY_STATE = 0X00;
/** status for card in the idle state */
const uint8_t R1_IDLE_STATE = 0X01;
/** status bit for illegal command */
const uint8_t R1_ILLEGAL_COMMAND = 0X04;
/** start data token for read or write single block*/
const uint8_t DATA_START_BLOCK = 0XFE;
/** stop token for write multiple blocks*/
const uint8_t STOP_TRAN_TOKEN = 0XFD;
/** start data token for write multiple blocks*/
const uint8_t WRITE_MULTIPLE_TOKEN = 0XFC;
/** mask for data response tokens after a write block operation */
const uint8_t DATA_RES_MASK = 0X1F;
/** write data accepted token */
const uint8_t DATA_RES_ACCEPTED = 0X05;
//==============================================================================
/**
 * \class CID
 * \brief Card IDentification (CID) register.
 */
typedef struct CID {
  // byte 0
  /** Manufacturer ID */
  unsigned char mid;
  // byte 1-2
  /** OEM/Application ID */
  char oid[2];
  // byte 3-7
  /** Product name */
  char pnm[5];
  // byte 8
  /** Product revision least significant digit */
  unsigned char prv_m : 4;
  /** Product revision most significant digit */
  unsigned char prv_n : 4;
  // byte 9-12
  /** Product serial number */
  uint32_t psn;
  // byte 13
  /** Manufacturing date year low digit */
  unsigned char mdt_year_high : 4;
  /** not used */
  unsigned char reserved : 4;
  // byte 14
  /** Manufacturing date month */
  unsigned char mdt_month : 4;
  /** Manufacturing date year low digit */
  unsigned char mdt_year_low : 4;
  // byte 15
  /** not used always 1 */
  unsigned char always1 : 1;
  /** CRC7 checksum */
  unsigned char crc : 7;
} __attribute__((packed)) cid_t;
//==============================================================================
/**
 * \class CSDV1
 * \brief CSD register for version 1.00 cards .
 */
typedef struct CSDV1 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  unsigned char taac;
  // byte 2
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  unsigned char c_size_high : 2;
  unsigned char reserved2 : 2;
  unsigned char dsr_imp : 1;
  unsigned char read_blk_misalign : 1;
  unsigned char write_blk_misalign : 1;
  unsigned char read_bl_partial : 1;
  // byte 7
  unsigned char c_size_mid;
  // byte 8
  unsigned char vdd_r_curr_max : 3;
  unsigned char vdd_r_curr_min : 3;
  unsigned char c_size_low : 2;
  // byte 9
  unsigned char c_size_mult_high : 2;
  unsigned char vdd_w_cur_max : 3;
  unsigned char vdd_w_curr_min : 3;
  // byte 10
  unsigned char sector_size_high : 6;
  unsigned char erase_blk_en : 1;
  unsigned char c_size_mult_low : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  unsigned char sector_size_low : 1;
  // byte 12
  unsigned char write_bl_len_high : 2;
  unsigned char r2w_factor : 3;
  unsigned char reserved3 : 2;
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved4 : 5;
  unsigned char write_partial : 1;
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved5: 2;
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Indicates the file format on the card */
  unsigned char file_format_grp : 1;
  // byte 15
  unsigned char always1 : 1;
  unsigned char crc : 7;
} __attribute__((packed)) csd1_t;
//==============================================================================
/**
 * \class CSDV2
 * \brief CSD register for version 2.00 cards.
 */
typedef struct CSDV2 {
  // byte 0
  unsigned char reserved1 : 6;
  unsigned char csd_ver : 2;
  // byte 1
  /** fixed to 0X0E */
  unsigned char taac;
  // byte 2
  /** fixed to 0 */
  unsigned char nsac;
  // byte 3
  unsigned char tran_speed;
  // byte 4
  unsigned char ccc_high;
  // byte 5
  /** This field is fixed to 9h, which indicates READ_BL_LEN=512 Byte */
  unsigned char read_bl_len : 4;
  unsigned char ccc_low : 4;
  // byte 6
  /** not used */
  unsigned char reserved2 : 4;
  unsigned char dsr_imp : 1;
  /** fixed to 0 */
  unsigned char read_blk_misalign : 1;
  /** fixed to 0 */
  unsigned char write_blk_misalign : 1;
  /** fixed to 0 - no partial read */
  unsigned char read_bl_partial : 1;
  // byte 7
  /** high part of card size */
  unsigned char c_size_high : 6;
  /** not used */
  unsigned char reserved3 : 2;
  // byte 8
  /** middle part of card size */
  unsigned char c_size_mid;
  // byte 9
  /** low part of card size */
  unsigned char c_size_low;
  // byte 10
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_high : 6;
  /** fixed to 1 - erase single is supported */
  unsigned char erase_blk_en : 1;
  /** not used */
  unsigned char reserved4 : 1;
  // byte 11
  unsigned char wp_grp_size : 7;
  /** sector size is fixed at 64 KB */
  unsigned char sector_size_low : 1;
  // byte 12
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_high : 2;
  /** fixed value of 2 */
  unsigned char r2w_factor : 3;
  /** not used */
  unsigned char reserved5 : 2;
  /** fixed value of 0 - no write protect groups */
  unsigned char wp_grp_enable : 1;
  // byte 13
  unsigned char reserved6 : 5;
  /** always zero - no partial block read*/
  unsigned char write_partial : 1;
  /** write_bl_len fixed for 512 byte blocks */
  unsigned char write_bl_len_low : 2;
  // byte 14
  unsigned char reserved7: 2;
  /** Do not use always 0 */
  unsigned char file_format : 2;
  unsigned char tmp_write_protect : 1;
  unsigned char perm_write_protect : 1;
  unsigned char copy : 1;
  /** Do not use always 0 */
  unsigned char file_format_grp : 1;
  // byte 15
  /** not used always 1 */
  unsigned char always1 : 1;
  /** checksum */
  unsigned char crc : 7;
} __attribute__((packed)) csd2_t;
//==============================================================================
/**
 * \class csd_t
 * \brief Union of old and new style CSD register.
 */
union csd_t {
  csd1_t v1;
  csd2_t v2;
};
#endif  // SdInfo_h
