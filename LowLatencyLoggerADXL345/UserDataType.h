#ifndef UserDataType_h
#define UserDataType_h
const uint8_t ACCEL_DIM = 3;
struct data_t {
  unsigned long time;
  int16_t accel[ACCEL_DIM];
};
#endif  // UserDataType_h
