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
 * \brief SpiDriver classes for Arduino compatible systems.
 */
#ifndef SdSpArduinoDriver_h
#define SdSpArduinoDriver_h
#include "../common/SysCall.h"
//------------------------------------------------------------------------------
/** SPISettings for SCK frequency in Hz. */
#define SD_SCK_HZ(maxSpeed) SPISettings(maxSpeed, MSBFIRST, SPI_MODE0)
/** SPISettings for SCK frequency in MHz. */
#define SD_SCK_MHZ(maxMhz) SPISettings(1000000UL*maxMhz, MSBFIRST, SPI_MODE0)
// SPI divisor constants
/** Set SCK to max rate. */
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
/** Set SCK for SD initialization - only needed for very old cards. */
#define SPI_LOW_SPEED SD_SCK_HZ(250000)
//------------------------------------------------------------------------------
#if SPI_DRIVER_SELECT == 2
class SoftSPIClass;
/** Port type for software SPI driver. */
typedef SoftSPIClass SpiPort_t;
#else  // SPI_DRIVER_SELECT
/** Port type for SPI hardware driver. */
typedef SPIClass SpiPort_t;
#endif  // SPI_DRIVER_SELECT
/**
 * \class SdSpiConfig
 * \brief SPI card configuration.
 */
class SdSpiConfig {
 public:
   /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   * \param[in] settings SPISettings.
   * \param[in] port The SPI port to use.
   */
  SdSpiConfig(uint8_t cs, uint8_t opt, SPISettings settings, SpiPort_t* port) :
    hsSettings(settings), csPin(cs), options(opt), spiPort(port) {}

  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   * \param[in] settings SPISettings.
   */
  SdSpiConfig(uint8_t cs, uint8_t opt, SPISettings settings) :
    hsSettings(settings), csPin(cs), options(opt), spiPort(nullptr) {}
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   */
  SdSpiConfig(uint8_t cs, uint8_t opt) :
    hsSettings(SPI_FULL_SPEED), csPin(cs), options(opt), spiPort(nullptr)  {}
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   */
  explicit SdSpiConfig(uint8_t cs) : hsSettings(SPI_FULL_SPEED), csPin(cs),
                                     options(SHARED_SPI), spiPort(nullptr) {}

  /** SPISettings after initialization. */
  const SPISettings hsSettings;

  /** Chip select pin. */
  const uint8_t csPin;
  /** Options */
  const uint8_t options;
  /** SPI port */
  SpiPort_t* spiPort;
};
//==============================================================================
#if SPI_DRIVER_SELECT == 0 && SD_HAS_CUSTOM_SPI
#define SD_ALT_SPI_DRIVER
/**
 * \class SdAltSpiDriver
 * \brief Optimized SPI class for access to SD and SDHC flash memory cards.
 */
class SdAltSpiDriver {
 public:
#if IMPLEMENT_SPI_PORT_SELECTION
  SdAltSpiDriver() : m_spi(nullptr) {}
#endif  // IMPLEMENT_SPI_PORT_SELECTION
  /** Activate SPI hardware. */
  void activate();
  /** deactivate SPI driver. */
  void end();
  /** Deactivate SPI hardware. */
  void deactivate();
  /** Initialize the SPI bus.
   *
   * \param[in] spiConfig SD card configuration.
   */
  void begin(SdSpiConfig spiConfig);
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive();
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] n Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t n);
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data);
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t n);
  /** Set CS low. */
  void select() {
    digitalWrite(m_csPin, LOW);
  }
  /** Save high speed SPISettings after SD initialization.
   *
   * \param[in] spiConfig SPI options.
   */
  void setHighSpeed(SdSpiConfig spiConfig) {
    m_spiSettings = spiConfig.hsSettings;
  }
  /** Set CS high. */
  void unselect() {
    digitalWrite(m_csPin, HIGH);
  }

 private:
#if IMPLEMENT_SPI_PORT_SELECTION || defined(DOXYGEN)
  SPIClass *m_spi;
#endif  // IMPLEMENT_SPI_PORT_SELECTION
  SPISettings m_spiSettings;
  uint8_t m_csPin;
};
typedef SdAltSpiDriver SdSpiDriver;
//------------------------------------------------------------------------------
// Use of in-line for AVR to save flash.
#if  defined(__AVR__) && ENABLE_ARDUINO_FEATURES
#define nop asm volatile ("nop\n\t")
//------------------------------------------------------------------------------
inline void SdAltSpiDriver::begin(SdSpiConfig spiConfig) {
  m_csPin = spiConfig.csPin;
  pinMode(m_csPin, OUTPUT);
  digitalWrite(m_csPin, HIGH);
  SPI.begin();
  m_spiSettings = SPI_LOW_SPEED;
}
//------------------------------------------------------------------------------
inline void SdAltSpiDriver::activate() {
  SPI.beginTransaction(m_spiSettings);
}
//------------------------------------------------------------------------------
inline void SdAltSpiDriver::deactivate() {
  SPI.endTransaction();
}
//------------------------------------------------------------------------------
inline uint8_t SdAltSpiDriver::receive() {
  return SPI.transfer(0XFF);
}
//------------------------------------------------------------------------------
inline uint8_t SdAltSpiDriver::receive(uint8_t* buf, size_t n) {
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
//------------------------------------------------------------------------------
inline void SdAltSpiDriver::send(uint8_t data) {
  SPI.transfer(data);
}
//------------------------------------------------------------------------------
inline void SdAltSpiDriver::send(const uint8_t* buf , size_t n) {
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
#endif  // __AVR__
//==============================================================================
#elif SPI_DRIVER_SELECT <= 1
/**
 * \class SdLibSpiDriver
 * \brief SdLibSpiDriver - use standard SPI library.
 */
class SdLibSpiDriver {
 public:
  /** Activate SPI hardware. */
  void activate() {
    m_spi->beginTransaction(m_spiSettings);
  }
  /** Initialize the SPI bus.
   *
   * \param[in] spiConfig SD card configuration.
   */
  void begin(SdSpiConfig spiConfig) {
    m_csPin = spiConfig.csPin;
    m_spiSettings = SPI_LOW_SPEED;

    if (spiConfig.spiPort) {
      m_spi = spiConfig.spiPort;
#if defined(SDCARD_SPI) && defined(SDCARD_SS_PIN)
    } else if (m_csPin == SDCARD_SS_PIN) {
      m_spi = &SDCARD_SPI;
#endif  // defined(SDCARD_SPI) && defined(SDCARD_SS_PIN)
    } else {
      m_spi = &SPI;
    }
    pinMode(m_csPin, OUTPUT);
    digitalWrite(m_csPin, HIGH);
    m_spi->begin();
  }
  /** Deactivate SPI hardware. */
  void deactivate() {
    m_spi->endTransaction();
  }
  /** Receive a byte.
   *
   * \return The byte.
   */
  uint8_t receive() {
    return m_spi->transfer( 0XFF);
  }
  /** Receive multiple bytes.
  *
  * \param[out] buf Buffer to receive the data.
  * \param[in] n Number of bytes to receive.
  *
  * \return Zero for no error or nonzero error code.
  */
  uint8_t receive(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      buf[i] = m_spi->transfer(0XFF);
    }
    return 0;
  }
  /** Send a byte.
   *
   * \param[in] data Byte to send
   */
  void send(uint8_t data) {
    m_spi->transfer(data);
  }
  /** Send multiple bytes.
   *
   * \param[in] buf Buffer for data to be sent.
   * \param[in] n Number of bytes to send.
   */
  void send(const uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      m_spi->transfer(buf[i]);
    }
  }
  /** Set CS low. */
  void select() {
    digitalWrite(m_csPin, LOW);
  }
  /** Save high speed SPISettings after SD initialization.
   *
   * \param[in] spiConfig SPI options.
   */
  void setHighSpeed(SdSpiConfig spiConfig) {
    m_spiSettings = spiConfig.hsSettings;
  }
  /** Set CS high. */
  void unselect() {
    digitalWrite(m_csPin, HIGH);
  }

 private:
  SPIClass* m_spi;
  SPISettings m_spiSettings;
  uint8_t m_csPin;
};
/** Use standard SPI library */
typedef SdLibSpiDriver SdSpiDriver;
//==============================================================================
#elif SPI_DRIVER_SELECT == 2
#define SD_SOFT_SPI_DRIVER
#include "../DigitalIO/SoftSPI.h"
class SoftSPIClass {
 public:
  virtual void begin() = 0;
  virtual uint8_t receive() = 0;
  virtual void send(uint8_t b) = 0;
};
//------------------------------------------------------------------------------
template<uint8_t MisoPin, uint8_t MosiPin, uint8_t SckPin>
class SoftSpiDriver : public SoftSPIClass {
 public:
  void begin() {m_spi.begin();}
  uint8_t receive() {return m_spi.receive();}
  void send(uint8_t b) {m_spi.send(b);}
 private:
  SoftSPI<MisoPin, MosiPin, SckPin, 0> m_spi;
};
//------------------------------------------------------------------------------
class SdSoftSpiDriver {
 public:
  void activate() {}
  void begin(SdSpiConfig spiConfig) {
    m_csPin = spiConfig.csPin;
    pinMode(m_csPin, OUTPUT);
    digitalWrite(m_csPin, HIGH);
    m_spi = spiConfig.spiPort ? spiConfig.spiPort : &m_null;
    m_spi->begin();
  }
  void deactivate() {}
  uint8_t receive() {return m_spi->receive();}
  uint8_t receive(uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      buf[i] = receive();
    }
    return 0;
  }
  void send(uint8_t data) {m_spi->send(data);}
  void send(const uint8_t* buf, size_t n) {
    for (size_t i = 0; i < n; i++) {
      send(buf[i]);
    }
  }
  void setHighSpeed(SdSpiConfig spiConfig) {
    (void)spiConfig;
  }
  void select() {digitalWrite(m_csPin, LOW);}
  void unselect() {digitalWrite(m_csPin, HIGH);}

 private:
  uint8_t m_csPin;
  class : public SoftSPIClass {
    void begin() {}
    uint8_t receive() {return 0XFF;}
    void send(uint8_t b) {(void)b;}
  } m_null;
  SoftSPIClass* m_spi;
};
typedef SdSoftSpiDriver SdSpiDriver;
//==============================================================================
#else  // SPI_DRIVER_SELECT
#error invalid SPI_DRIVER_SELECT
#endif  // SPI_DRIVER_SELECT
#endif  // SdSpArduinoDriver_h
