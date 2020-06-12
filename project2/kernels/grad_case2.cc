#include "../run2.h"

void grad_case2(float (&dB) [4][16], float (&dA)[4][16]) {
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 16; ++j) {
      dA[i][j] = ((dB[i][j] * dB[i][j]) + 0.0);
    }
  }
}

