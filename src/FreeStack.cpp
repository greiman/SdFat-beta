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
#include "FreeStack.h"
#if HAS_UNUSED_STACK
#ifdef __AVR__
inline char* heapEnd() {
  return __brkval ? __brkval : &__bss_end;
}
inline char* stackPointer() {
  return reinterpret_cast<char*>(SP);
}
#elif defined(__arm__)
inline char* heapEnd() {
  return reinterpret_cast<char*>(sbrk(0));
}
inline char* stackPointer() {
  register uint32_t sp asm("sp");
  return reinterpret_cast<char*>(sp);
}
#endif  // #elif define(__arm__)
//------------------------------------------------------------------------------
/** Stack fill pattern. */
const char FILL = 0x55;
void FillStack() {
  char* p = heapEnd();
  char* top = stackPointer();
  while (p < top) {
    *p++ = FILL;
  }
}
//------------------------------------------------------------------------------
// May fail if malloc or new is used.
int UnusedStack() {
  char* h = heapEnd();
  char* top = stackPointer();
  int n;

  for (n = 0; (h + n) < top; n++) {
    if (h[n] != FILL) {
      if (n >= 16) {
        break;
      }
      // Attempt to skip used heap.
      h += n;
      n = 0;
    }
  }
  return n;
}
#endif  // HAS_UNUSED_STACK
