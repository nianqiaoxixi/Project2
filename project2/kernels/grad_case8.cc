#include "../run2.h"

void grad_case8(float (&dB)[32], float (&dA) [2][16]) {
  for (int i = 0; i < 2; ++i) {
    for (int _i = 0; _i < 16; ++_i) {
      float temp = 0;
      temp = (temp + dB[(i * 16 + _i)]);
      dA[i][_i] = temp;
    }
  }
}

