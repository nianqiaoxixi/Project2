#include "../run.h"

void kernel_case7(float (&A)[32][16], float (&B) [16][32]) {
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 32; ++j) {
      float temp0 = 0;
      temp0 = (temp0 + A[j][i]);
      B[i][j] = temp0;
    }
  }
}

