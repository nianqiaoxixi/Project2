#include "../run.h"

<<<<<<< HEAD
void kernel_case7(float (&A)[32][16], float (&B) [16][32]) {
  for (int i = 0; i < 16; ++i) {
    for (int j = 0; j < 32; ++j) {
      float temp0 = 0;
      temp0 = (temp0 + A[j][i]);
      B[i][j] = temp0;
    }
  }
}

=======
void kernel_case7(float (&A)[32][16], float (&B)[16][32]) {}
>>>>>>> 3e5ac3ccec1e739f7c0e410878734b437e4372c1
