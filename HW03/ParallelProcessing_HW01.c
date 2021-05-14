#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX 1000
#define EPOCH 100
#pragma warning(disable : 4996)

void addVector(int *C, int *A, int *B) {
  static int idx = 0;
  C[idx % MAX] = A[idx % MAX] + B[idx % MAX];
  idx++;
}

int main() {
  int A[MAX], B[MAX], C[MAX];
  float time_avg = 0;
  clock_t start, end;

  for (int i = 0; i < MAX; i++) {
    A[i] = i;
    B[i] = i * i;
  }

  for (int t = 0; t < EPOCH; t++) { // EPOCH만큼 반복

    int *ptr_A = 0; // 비슷한 방식으로 진행하였을 때 걸리는 시간 차이를
    int *ptr_B = 0; // 보여주기 위해 유사한 방식으로 프로그램을 작성
    int *ptr_C = 0;
    ptr_A = (int *)malloc(sizeof(int) * MAX);
    ptr_B = (int *)malloc(sizeof(int) * MAX);
    ptr_C = (int *)malloc(sizeof(int) * MAX);

    memcpy(ptr_A, A, sizeof(int) * MAX); // 메모리에 복사하여 계산하는
    memcpy(ptr_B, B, sizeof(int) * MAX); // 조건도 추가하고 측정하여 비교

    start = clock();
    for (int i = 0; i < MAX; i++) {
      addVector(ptr_C, ptr_A, ptr_B);
    }
    end = clock();

    memcpy(C, ptr_C, sizeof(int) * MAX);

    int cycle = MAX / 10;
    for (int i = 0; i < MAX; i++) {
      if (i % cycle == 0) {
        printf("%d + %d = %d\n", A[i], B[i], C[i]);
      }
    }

    time_avg += (float)(end - start);

    printf("\n\n");
    free(ptr_A);
    free(ptr_B);
    free(ptr_C);
  }

  printf("time : %f\n", time_avg / EPOCH);
  return 0;
}
