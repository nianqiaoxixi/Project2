#include "../run2.h"

void grad_case3(float (&B)[16][16], float (&dC)[4][16], float (&dA) [4][16]) {
  for (int i = 0; i < 4; ++i) {
    for (int k = 0; k < 16; ++k) {
      float temp = 0;
      for (int j = 0; j < 16; ++j) {
        temp = (temp + dC[i][j] * B[k][j]);
      }
      dA[i][k] = temp;
    }
  }
}

