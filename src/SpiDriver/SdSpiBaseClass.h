/**
 * Copyright (c) 2011-2019 Bill Greiman
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
/**
 * \file
 * \brief Base class for external SPI driver.
 */
#ifndef SdSpiBaseClass_h
#define SdSpiBaseClass_h
/**
 * \class SdSpiBaseClass
 * \brief Base class for external SPI drivers
 */
class SdSpiBaseClass {
 public:
  /** Activate SPI hardware. */
  virtual void activate() {}
  /** Initialize the SPI bus. */
  virtual void begin() = 0;
  /** Deactivate SPI hardware. */
  virtual void deactivate() {}
  /** Receive a byte.
   *
   * \return The byte.
   */
  virtual uint8_t receive() = 0;
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] count Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  virtual uint8_t receive(uint8_t* buf, size_t count) = 0;
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  virtual void send(uint8_t data) = 0;
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] count Number of bytes to send.
   */
  virtual void send(const uint8_t* buf, size_t count) = 0;
  /** Save high speed SPISettings after SD initialization.
   *
   * \param[in] maxSck Maximum SCK frequency.
   */
  virtual void setSckSpeed(uint32_t maxSck) {(void)maxSck;}
};
#include "SPI.h"
/**
 * \class MySpiClass
 * \brief Sample external SPI class.
 */
class MySpiClass : public SdSpiBaseClass {
 public:
  /** Activate SPI hardware. */
  void activate() {
    SPI.beginTransaction(m_spiSettings);
  }
  /** Initialize the SPI bus. */
  void begin() {SPI.begin();}
  /** Deactivate SPI hardware. */
  void deactivate() {SPI.endTransaction();}
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {return SPI.transfer(0XFF);}
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] count Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      buf[i] = SPI.transfer(0XFF);
    }
    return 0;
  }
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data) {SPI.transfer(data);}
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] count Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t count) {
    for (size_t i = 0; i < count; i++) {
      SPI.transfer(buf[i]);
    }
  }
  /** Save high speed SPISettings after SD initialization.
   *
   * \param[in] maxSck Maximum SCK frequency.
   */
  void setSckSpeed(uint32_t maxSck) {
    m_spiSettings = SPISettings(maxSck, MSBFIRST, SPI_MODE0);
  }

 private:
  SPISettings m_spiSettings;
};
#endif  // SdSpiBaseClass_h
