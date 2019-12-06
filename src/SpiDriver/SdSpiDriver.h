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
 * \brief SpiDriver classes
 */
#ifndef SdSpiDriver_h
#define SdSpiDriver_h
#include "../common/SysCall.h"
/**
 * Initialize SD chip select pin.
 *
 * \param[in] pin SD card chip select pin.
 */
void sdCsInit(SdCsPin_t pin);
/**
 * Initialize SD chip select pin.
 *
 * \param[in] pin SD card chip select pin.
 * \param[in] level SD card chip select level.
 */
void sdCsWrite(SdCsPin_t pin, bool level);
//------------------------------------------------------------------------------
/** SPISettings for SCK frequency in Hz. */
#define SD_SCK_HZ(maxSpeed) (maxSpeed)
/** SPISettings for SCK frequency in MHz. */
#define SD_SCK_MHZ(maxMhz) (1000000UL*(maxMhz))
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
//------------------------------------------------------------------------------
/** The SD is the only device on the SPI bus. */
#define DEDICATED_SPI 0X80
/** SPI bus is share with other devices. */
#define SHARED_SPI 0
#if SPI_DRIVER_SELECT < 2
#include "SPI.h"
/** Port type for SPI hardware driver. */
typedef SPIClass SpiPort_t;
#elif SPI_DRIVER_SELECT == 2
class SoftSPIClass;
/** Port type for software SPI driver. */
typedef SoftSPIClass SpiPort_t;
#else  // SPI_DRIVER_SELECT
#include "SdSpiBaseClass.h"
typedef SdSpiBaseClass  SpiPort_t;
#endif  // SPI_DRIVER_SELECT
//------------------------------------------------------------------------------
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
   * \param[in] maxSpeed Maximum SCK frequency.
   * \param[in] port The SPI port to use.
   */
  SdSpiConfig(SdCsPin_t cs, uint8_t opt, uint32_t maxSpeed, SpiPort_t* port) :
    csPin(cs), options(opt), maxSck(maxSpeed), spiPort(port) {}

  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   * \param[in] maxSpeed Maximum SCK frequency.
   */
  SdSpiConfig(SdCsPin_t cs, uint8_t opt, uint32_t maxSpeed) :
    csPin(cs), options(opt), maxSck(maxSpeed), spiPort(nullptr) {}
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   * \param[in] opt Options.
   */
  SdSpiConfig(SdCsPin_t cs, uint8_t opt) :
    csPin(cs), options(opt), maxSck(SPI_FULL_SPEED), spiPort(nullptr)  {}
  /** SdSpiConfig constructor.
   *
   * \param[in] cs Chip select pin.
   */
  explicit SdSpiConfig(SdCsPin_t cs) : csPin(cs), options(SHARED_SPI),
                                     maxSck(SPI_FULL_SPEED), spiPort(nullptr) {}

  /** Chip select pin. */
  const SdCsPin_t csPin;
  /** Options */
  const uint8_t options;
  /** Max SCK frequency */
  const uint32_t maxSck;
  /** SPI port */
  SpiPort_t* spiPort;
};

#if SPI_DRIVER_SELECT < 2
#include "SdSpiArduinoDriver.h"
#elif SPI_DRIVER_SELECT == 2
#include "SdSpiSoftDriver.h"
#elif SPI_DRIVER_SELECT == 3
#include "SdSpiExternalDriver.h"
#elif SPI_DRIVER_SELECT == 4
#include "SdSpiBareUnoDriver.h"
typedef SdSpiDriverBareUno SdSpiDriver;
#else  // SPI_DRIVER_SELECT
#error Invalid SPI_DRIVER_SELECT
#endif  // SPI_DRIVER_SELECT
#endif  // SdSpiDriver_h
