#include "../run.h"

<<<<<<< HEAD
void kernel_case1(float (&A) [32][16]) {
  for (int i = 0; i < 32; ++i) {
    for (int j = 0; j < 16; ++j) {
      float temp0 = 0;
      temp0 = (temp0 + 2);
      A[i][j] = temp0;
    }
  }
}

=======
void kernel_case1(float (&A)[32][16]) {}
>>>>>>> 3e5ac3ccec1e739f7c0e410878734b437e4372c1
