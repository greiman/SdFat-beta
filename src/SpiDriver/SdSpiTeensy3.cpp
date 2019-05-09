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
#include "SdSpiDriver.h"
#if defined(SD_ALT_SPI_DRIVER) &&  defined(__arm__) && defined(CORE_TEENSY)
#define USE_BLOCK_TRANSFER 1
//------------------------------------------------------------------------------
void SdAltSpiDriver::activate() {
  m_spi->beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
void SdAltSpiDriver::begin(SdSpiConfig spiConfig) {
  m_csPin = spiConfig.csPin;
  m_spiSettings = SPI_LOW_SPEED;
  if (spiConfig.spiPort) {
    m_spi = spiConfig.spiPort;
#if defined(SDCARD_SPI) && defined(SDCARD_SS_PIN)
  } else if (m_csPin == SDCARD_SS_PIN) {
    m_spi = &SDCARD_SPI;
    m_spi->setMISO(SDCARD_MISO_PIN);
    m_spi->setMOSI(SDCARD_MOSI_PIN);
    m_spi->setSCK(SDCARD_SCK_PIN);
#endif  // defined(SDCARD_SPI) && defined(SDCARD_SS_PIN)
  } else {
    m_spi = &SPI;
  }
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  m_spi->begin();
}
//------------------------------------------------------------------------------
void SdAltSpiDriver::deactivate() {
  m_spi->endTransaction();
}
//------------------------------------------------------------------------------
/** Receive a byte.
 *
 * \return The byte.
 */
uint8_t SdAltSpiDriver::receive() {
  return m_spi->transfer(0XFF);
}
/** Receive multiple bytes.
 *
 * \param[out] buf Buffer to receive the data.
 * \param[in] n Number of bytes to receive.
 *
 * \return Zero for no error or nonzero error code.
 */
uint8_t SdAltSpiDriver::receive(uint8_t* buf, size_t n) {
#if USE_BLOCK_TRANSFER
  memset(buf, 0XFF, n);
  m_spi->transfer(buf, n);
#else  // USE_BLOCK_TRANSFER
  for (size_t i = 0; i < n; i++) {
    buf[i] = m_spi->transfer(0XFF);
  }
#endif  // USE_BLOCK_TRANSFER
  return 0;
}
/** Send a byte.
 *
 * \param[in] b Byte to send
 */
void SdAltSpiDriver::send(uint8_t b) {
  m_spi->transfer(b);
}
/** Send multiple bytes.
 *
 * \param[in] buf Buffer for data to be sent.
 * \param[in] n Number of bytes to send.
 */
void SdAltSpiDriver::send(const uint8_t* buf , size_t n) {
#if USE_BLOCK_TRANSFER
  uint32_t tmp[128];
  if (0 < n && n <= 512) {
    memcpy(tmp, buf, n);
    m_spi->transfer(tmp, n);
    return;
  }
#endif  // USE_BLOCK_TRANSFER
  for (size_t i = 0; i < n; i++) {
    m_spi->transfer(buf[i]);
  }
}
#endif  // defined(SD_ALT_SPI_DRIVER) && defined(__arm__) &&defined(CORE_TEENSY)
