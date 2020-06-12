#include "../run2.h"

void grad_case10(float (&dA) [8][8], float (&dB)[10][8]) {
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      for (int h = 0; h < 10; ++h) {
        for (int w = 0; w < 10; ++w) {
          dB[w][j] = ((dA[i][j] + (dA[i][j] + dA[i][j])) / 3);
        }
      }
    }
  }
}

