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

/** The SD is the only device on the SPI bus. */
#define DEDICATED_SPI 0X80
/** SPI bus is share with other devices. */
#define SHARED_SPI 0

#if ENABLE_ARDUINO_FEATURES || defined(DOXYGEN)
#include "SdSpiArduinoDriver.h"
#else  // ENABLE_ARDUINO_FEATURES
#include "SdSpiBareUnoDriver.h"
typedef SdSpiDriverBareUno SdSpiDriver;
#endif  // ENABLE_ARDUINO_FEATURES
#endif  // SdSpiDriver_h
