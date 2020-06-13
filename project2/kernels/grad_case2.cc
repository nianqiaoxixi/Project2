#include "../run2.h"

void grad_case2(float (&A)[4][16], float (&dB)[4][16], float (&dA) [4][16]) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 16; ++j) {
      float temp = 0;
      temp = (temp + (dB[i][j] * (A[i][j] + A[i][j]) + 0.0));
      dA[i][j] = temp;
    }
  }
}

