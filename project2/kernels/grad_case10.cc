#include "../run2.h"

void grad_case10(float (&dA)[8][8], float (&dB) [10][8]) {
  for (int j = 0; j < 8; ++j) {
    for (int h = 0; h < 10; ++h) {
      float temp = 0;
      temp = (temp + (((h >= 0) && (h < 8)? dA[h][j]: 0) + ((((h - 1) >= 0) && ((h - 1) < 8)? dA[(h - 1)][j]: 0) + (((h - 2) >= 0) && ((h - 2) < 8)? dA[(h - 2)][j]: 0))) / 3);
      dB[h][j] = temp;
    }
  }
}

