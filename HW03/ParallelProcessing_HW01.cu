#include <cuda.h>
#include <stdio.h>
#define MAX 1000
#define EPOCH 10

__global__ void addVector(int *c, int *a, int *b) {
  int i = threadIdx.x;
  c[i] = a[i] + b[i];
}

int main() {
  int a[MAX], b[MAX], c[MAX];
  float time, time_avg = 0;
  cudaEvent_t start, stop;

  cudaEventCreate(&start);
  cudaEventCreate(&stop);
  cudaEventRecord(start, 0);

  for (int i = 0; i < MAX; i++) {
    a[i] = i;
    b[i] = i * i;
  }

  for (int t = 0; t < EPOCH; t++) {
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);

    int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
    cudaMalloc((void **)&dev_a, MAX * sizeof(int));
    cudaMalloc((void **)&dev_b, MAX * sizeof(int));
    cudaMalloc((void **)&dev_c, MAX * sizeof(int));

    cudaMemcpy(dev_a, a, MAX * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(dev_b, b, MAX * sizeof(int), cudaMemcpyHostToDevice);
    addVector<<<1, MAX>>>(dev_c, dev_a, dev_b);
    cudaMemcpy(c, dev_c, MAX * sizeof(int), cudaMemcpyDeviceToHost);

    for (int i = 0; i < MAX; i++) {
      if (i % 100 == 0) {
        printf("%d + %d = %d\n", a[i], b[i], c[i]);
      }
    }

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    time_avg += time;
    printf("\n\n");
  }

  printf("time : %f\n", time_avg / EPOCH);
  return 0;
}