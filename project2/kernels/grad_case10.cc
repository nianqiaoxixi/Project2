#include "../run2.h"

void grad_case10(float (&dA)[8][8], float (&dB) [10][8]) {
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      for (int h = 0; h < 10; ++h) {
        float temp = 0;
        temp = (temp + (dA[(h - 1)][j] + (dA[(h - 1)][j] + dA[(h - 1)][j])) / 3);
        dB[h][j] = temp;
      }
    }
  }
}

