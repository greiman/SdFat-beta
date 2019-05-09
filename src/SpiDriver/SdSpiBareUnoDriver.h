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
#ifndef SdSpiBareUnoDriver_h
#define SdSpiBareUnoDriver_h
/**
 * \file
 * \brief Driver to test with no Arduino includes.
 */

#include <avr/interrupt.h>
#include "../common/SysCall.h"
#define nop asm volatile ("nop\n\t")
#ifndef HIGH
#define HIGH 1
#endif  // HIGH
#ifndef LOW
#define LOW 0
#endif  // LOW
#ifndef INPUT
#define INPUT 0
#endif  // INPUT
#ifndef OUTPUT
#define OUTPUT 1
#endif  // OUTPUT

inline uint8_t unoBit(uint8_t pin) {
  return 1 << (pin < 8 ? pin : pin < 14 ? pin - 8 : pin - 14);
}
inline uint8_t unoDigitalRead(uint8_t pin) {
  volatile uint8_t* reg = pin < 8 ? &PIND : pin < 14 ? &PINB : &PINC;
  return *reg & unoBit(pin);
}
inline void unoDigitalWrite(uint8_t pin, uint8_t value) {
  volatile uint8_t* port = pin < 8 ? &PORTD : pin < 14 ? &PORTB : &PORTC;
  uint8_t bit = unoBit(pin);
  cli();
  if (value) {
    *port |= bit;
  } else {
    *port &= ~bit;
  }
  sei();
}

inline void unoPinMode(uint8_t pin, uint8_t mode) {
  uint8_t bit = unoBit(pin);
  volatile uint8_t* reg = pin < 8 ? &DDRD : pin < 14 ? &DDRB : &DDRC;

  cli();
  if (mode == OUTPUT) {
    *reg |= bit;
  } else {
    *reg &= ~bit;
    // handle INPUT pull-up
    unoDigitalWrite(pin, mode != INPUT);
  }
  sei();
}

class SdSpiConfig {
 public:
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   */
  SdSpiConfig(uint8_t cs, uint8_t opt) :
    csPin(cs), options(opt) {}
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   */
  explicit SdSpiConfig(uint8_t cs) :
    csPin(cs), options(SHARED_SPI) {}

  /** Chip select pin. */
  const uint8_t csPin;
  /** Options */
  const uint8_t options;
};

#define UNO_SS   10
#define UNO_MOSI 11
#define UNO_MISO 12
#define UNO_SCK  13
//------------------------------------------------------------------------------
/**
 * \class SdSpiDriverBareUno
 * \brief Optimized SPI class for access to SD and SDHC flash memory cards.
 */
class SdSpiDriverBareUno {
 public:
  /** Activate SPI hardware. */
  void activate() {}
  /** deactivate SPI driver. */
  void end() {}
  /** Deactivate SPI hardware. */
  void deactivate() {}
  /** Initialize the SPI bus.
   *
   * \param[in] spiConfig SD card configuration.
   */
  void begin(SdSpiConfig spiConfig) {
    m_csPin = spiConfig.csPin;
    unoPinMode(m_csPin, OUTPUT);
    unoDigitalWrite(m_csPin, HIGH);
    unoDigitalWrite(UNO_SS, HIGH);
    unoPinMode(UNO_SS, OUTPUT);
    SPCR |= _BV(MSTR);
    SPCR |= _BV(SPE);
    SPSR = 0;
    unoPinMode(UNO_SCK, OUTPUT);
    unoPinMode(UNO_MOSI, OUTPUT);
  }
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {
    return transfer(0XFF);
  }
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] n Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t n) {
    if (n == 0) {
      return 0;
    }
    uint8_t* pr = buf;
    SPDR = 0XFF;
    while (--n > 0) {
      while (!(SPSR & _BV(SPIF))) {}
      uint8_t in = SPDR;
      SPDR = 0XFF;
      *pr++ = in;
      // nops to optimize loop for 16MHz CPU 8 MHz SPI
      nop;
      nop;
    }
    while (!(SPSR & _BV(SPIF))) {}
    *pr = SPDR;
    return 0;
  }
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data) {
    transfer(data);
  }
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t n) {
    if (n == 0) {
      return;
    }
    SPDR = *buf++;
    while (--n > 0) {
      uint8_t b = *buf++;
      while (!(SPSR & (1 << SPIF))) {}
      SPDR = b;
      // nops to optimize loop for 16MHz CPU 8 MHz SPI
      nop;
      nop;
    }
    while (!(SPSR & (1 << SPIF))) {}
  }
  /** Set CS low. */
  void select() {
     unoDigitalWrite(m_csPin, LOW);
  }
  /** Save high speed SPISettings after SD initialization.
   *
   * \param[in] spiConfig SPI options.
   */
  void setHighSpeed(SdSpiConfig spiConfig) {
    (void)spiConfig;
    SPSR |= 1 << SPI2X;
//    m_spiSettings = spiConfig.hsSettings;
  }
  static uint8_t transfer(uint8_t data) {
    SPDR = data;
    while (!(SPSR & _BV(SPIF))) {}  // wait
    return SPDR;
  }
  /** Set CS high. */
  void unselect() {
    unoDigitalWrite(m_csPin, HIGH);
  }

 private:
  uint8_t m_csPin;
};
#endif  // SdSpiBareUnoDriver_h
