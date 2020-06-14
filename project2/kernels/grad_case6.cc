#include "../run2.h"

void grad_case6(float (&C)[8][16][3][3], float (&dA)[2][8][5][5], float (&dB) [2][16][7][7]) {
  for (int n = 0; n < 2; ++n) {
    for (int c = 0; c < 16; ++c) {
      for (int h = 0; h < 7; ++h) {
        for (int w = 0; w < 7; ++w) {
          float temp = 0;
          for (int k = 0; k < 8; ++k) {
            for (int p = 0; p < 5; ++p) {
              for (int q = 0; q < 5; ++q) {
                temp = (temp + (((h - p) >= 0) && ((h - p) < 3) && ((w - q) >= 0) && ((w - q) < 3)? dA[n][k][p][q] * C[k][c][(h - p)][(w - q)]: 0));
              }
            }
          }
          dB[n][c][h][w] = temp;
        }
      }
    }
  }
}

