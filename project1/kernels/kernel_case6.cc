#include "../run.h"

void kernel_case6(float (&B)[2][16][7][7], float (&C)[8][16][3][3], float (&A) [2][8][5][5]) {
  for (int n = 0; n < 2; ++n) {
    for (int k = 0; k < 8; ++k) {
      for (int p = 0; p < 5; ++p) {
        for (int q = 0; q < 5; ++q) {
          float temp0 = 0;
          float temp1 = 0;
          temp0 = (temp0 + A[n][k][p][q]);
          for (int c = 0; c < 16; ++c) {
            for (int r = 0; r < 3; ++r) {
              for (int s = 0; s < 3; ++s) {
                if ((q + s) < 7) {
                  if ((p + r) < 7) {
                    temp1 = (temp1 + (B[n][c][(p + r)][(q + s)] * C[k][c][r][s]));
                  } else {
                    temp1 = temp1;
                  }
                } else {
                  temp1 = temp1;
                }
              }
            }
          }
          A[n][k][p][q] = (temp0 + temp1);
        }
      }
    }
  }
}

