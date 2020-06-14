#include "../run2.h"

void grad_case6(float (&C)[8][16][3][3], float (&dA)[2][8][5][5], float (&dB) [2][16][7][7]) {
  for (int n = 0; n < 2; ++n) {
    for (int c = 0; c < 16; ++c) {
      for (int _p = 0; _p < 7; ++_p) {
        for (int _q = 0; _q < 7; ++_q) {
          float temp = 0;
          for (int k = 0; k < 8; ++k) {
            for (int p = 0; p < 5; ++p) {
              for (int q = 0; q < 5; ++q) {
                temp = (temp + (((_p - p) >= 0) && ((_p - p) < 3) && ((_q - q) >= 0) && ((_q - q) < 3)? dA[n][k][p][q] * C[k][c][(_p - p)][(_q - q)]: 0));
              }
            }
          }
          dB[n][c][_p][_q] = temp;
        }
      }
    }
  }
}

