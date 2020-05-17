#include "../run.h"

void kernel_case10(float (&B)[10][10], float (&A) [8][8]) {
  for (int i = 0; i < 8; ++i) {
    for (int j = 0; j < 8; ++j) {
      float temp0 = 0;
      if ((i + 2) < 10) {
        if ((i + 1) < 10) {
          temp0 = (temp0 + ((B[i][j] + (B[(i + 1)][j] + B[(i + 2)][j])) / 3));
        } else {
          temp0 = temp0;
        }
      } else {
        temp0 = temp0;
      }
      if ((i + 2) < 10) {
        if ((i + 1) < 10) {
          A[i][j] = temp0;
        } else {
          A[i][j] = A[i][j];
        }
      } else {
        A[i][j] = A[i][j];
      }
    }
  }
}

