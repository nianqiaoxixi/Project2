#include "../run2.h"

void grad_case9(float (&dB)[4][6], float (&dA) [4]) {
  for (int i = 0; i < 4; ++i) {
    float temp = 0;
    for (int j = 0; j < 6; ++j) {
      temp = (temp + dB[i][j]);
    }
    dA[i] = temp;
  }
}

