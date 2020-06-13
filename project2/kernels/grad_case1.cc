#include "../run2.h"

void grad_case1(float (&B)[4][16], float (&dC)[4][16], float (&dA) [4][16]) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 16; ++j) {
      float temp = 0;
      temp = (temp + (dC[i][j] * B[i][j] + 0.0));
      dA[i][j] = temp;
    }
  }
}

