#ifndef SystemInclude_h
#define SystemInclude_h
#if defined(ARDUINO)
#include <Arduino.h>
#include <SPI.h>
#elif defined(PLATFORM_ID)  //Only defined if a Particle device
#include "application.h"
#else  // 
#error "Unknown system"
#endif  // defined(ARDUINO)
#ifndef F
#define F(str) (str)
#endif  // F
#endif  // SystemInclude_h
