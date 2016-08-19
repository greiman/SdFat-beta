#include "UserTypes.h"
// User data functions.  Modify these functions for your data items.

// Start time for data
uint32_t startTime;

// Acquire a data record.
void acquireData(data_t* data) {
  data->time = micros();
  for (int i = 0; i < ADC_DIM; i++) {
    data->adc[i] = analogRead(i);
  }
}

// Print a data record.
void printData(Print* pr, data_t* data) {
  if (startTime == 0) {
    startTime = data->time;
  }
  pr->print(data->time - startTime);
  for (int i = 0; i < ADC_DIM; i++) {
    pr->write(',');
    pr->print(data->adc[i]);
  }
  pr->println();
}

// Print data header.
void printHeader(Print* pr) {
  startTime = 0;
  pr->print(F("micros"));
  for (int i = 0; i < ADC_DIM; i++) {
    pr->print(F(",adc"));
    pr->print(i);
  }
  pr->println();
}

// Sensor setup
void userSetup() {
}
