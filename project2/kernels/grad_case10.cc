#include "../run2.h"

void grad_case10(float (&dA)[8][8], float (&dB) [10][8]) {
  for (int j = 0; j < 8; ++j) {
    for (int _i = 0; _i < 10; ++_i) {
      float temp = 0;
      temp = (temp + (((_i >= 0) && (_i < 8)? dA[_i][j]: 0) + ((((_i - 1) >= 0) && ((_i - 1) < 8)? dA[(_i - 1)][j]: 0) + (((_i - 2) >= 0) && ((_i - 2) < 8)? dA[(_i - 2)][j]: 0))) / 3);
      dB[_i][j] = temp;
    }
  }
}

