/* Arduino SdSpi Library
 * Copyright (C) 2013 by William Greiman
 *
 * STM32F1 code for Maple and Maple Mini support, 2015 by Victor Perez
 *
 * This file is part of the Arduino SdSpi Library
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
 * along with the Arduino SdSpi Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
#if defined(__STM32F1__)
#include "SdSpi.h"
#define USE_STM32F1_DMAC 1

//------------------------------------------------------------------------------
void SdSpi::begin(uint8_t chipSelectPin) {
  pinMode(chipSelectPin, OUTPUT);
  digitalWrite(chipSelectPin, HIGH);
  SPI.begin();
}
//------------------------------------------------------------------------------
//  initialize SPI controller STM32F1
void SdSpi::beginTransaction(uint8_t divisor) {
#if ENABLE_SPI_TRANSACTIONS
  SPISettings settings(F_CPU/(divisor ? divisor : 1), MSBFIRST, SPI_MODE0);
  SPI.beginTransaction(settings);
#else  // ENABLE_SPI_TRANSACTIONS
  SPI.setClockDivider(divisor);
  SPI.setBitOrder(MSBFIRST);
  SPI.setDataMode(SPI_MODE0);
#endif  // ENABLE_SPI_TRANSACTIONS
}
//------------------------------------------------------------------------------
void SdSpi::endTransaction() {
#if ENABLE_SPI_TRANSACTIONS
  SPI.endTransaction();
#endif  // ENABLE_SPI_TRANSACTIONS
}
//------------------------------------------------------------------------------
// should be valid for STM32
/** SPI receive a byte */
uint8_t SdSpi::receive() {
  return SPI.transfer(0XFF);
}
//------------------------------------------------------------------------------
/** SPI receive multiple bytes */
// check and finish.

uint8_t SdSpi::receive(uint8_t* buf, size_t n) {
  int rtn = 0;

#if USE_STM32F1_DMAC

  rtn = SPI.dmaTransfer(0, const_cast<uint8*>(buf), n);

#else  // USE_STM32F1_DMAC
  for (size_t i = 0; i < n; i++) {
    buf[i] = SPI.transfer (0xFF);
  }
#endif  // USE_STM32F1_DMAC
  return rtn;
}
//------------------------------------------------------------------------------
/** SPI send a byte */
void SdSpi::send(uint8_t b) {
  SPI.transfer(b);
}
//------------------------------------------------------------------------------
void SdSpi::send(const uint8_t* buf , size_t n) {

#if USE_STM32F1_DMAC
  SPI.dmaSend(const_cast<uint8*>(buf), n);
#else  // #if USE_STM32F1_DMAC
  SPI.write(buf, n);
#endif
}
#endif  // USE_NATIVE_STM32F1_SPI
