#include "../run.h"

<<<<<<< HEAD
void kernel_case5(float (&B)[16][32], float (&C)[32][32], float (&D)[16][32], float (&alpha), float (&beta), float (&A) [16][32]) {
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 32; ++j) {
      float temp0 = 0;
      float temp1 = 0;
      temp0 = (temp0 + A[i][j]);
      for (int k = 0; k < 32; ++k) {
        temp1 = (temp1 + (alpha * (B[i][k] * C[k][j])));
      }
      A[i][j] = (temp0 + temp1);
    }
  }
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 32; ++j) {
      float temp0 = 0;
      float temp1 = 0;
      temp0 = (temp0 + A[i][j]);
      temp1 = (temp1 + (beta * D[i][j]));
      A[i][j] = (temp0 + temp1);
    }
  }
}

=======
void kernel_case5(float (&B)[16][32], float (&C)[32][32], float (&D)[16][32], float &alpha, float &beta, float (&A)[16][32]) {}
>>>>>>> 3e5ac3ccec1e739f7c0e410878734b437e4372c1
