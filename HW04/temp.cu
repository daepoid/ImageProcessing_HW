#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <cuda.h>
#include <stdio.h>
// 32 64 128
#define MAX 128 // matrix length
#define BLOCKS 16
#define EPOCH 100
#pragma warning(disable : 4996)

__global__ void matrix_multiplication(double *C, double *A, double *B) {
  int row = blockIdx.y * blockDim.y + threadIdx.y;
  int col = blockIdx.x * blockDim.x + threadIdx.x;

  double sum = 0.0;
  if (row < MAX && col < MAX) {
    for (int i = 0; i < MAX; i++) {
      sum += A[MAX * row + i] * B[MAX * i + col];
    }
  }
  C[MAX * row + col] = sum;
}

int main() {
  double A[MAX * MAX], B[MAX * MAX], C[MAX * MAX];
  float time, time_avg = 0;
  cudaEvent_t start, stop;

  cudaEventCreate(&start);   // create start event
  cudaEventCreate(&stop);    // create stop event
  cudaEventRecord(start, 0); // record start event

  for (int i = 0; i < MAX * MAX; i++) { // create A, B data
    A[i] = 1;
    B[i] = 1;
    C[i] = 0;
  }

  for (int t = 0; t < EPOCH; t++) { // EPOCH만큼 반복하여 평균시간을 계산
    double *cuda_A = 0;
    double *cuda_B = 0;
    double *cuda_C = 0;
    cudaMalloc((void **)&cuda_A, MAX * MAX * sizeof(double));
    cudaMalloc((void **)&cuda_B, MAX * MAX * sizeof(double));
    cudaMalloc((void **)&cuda_C, MAX * MAX * sizeof(double));

    cudaMemcpy(cuda_A, A, MAX * MAX * sizeof(double), cudaMemcpyHostToDevice);
    cudaMemcpy(cuda_B, B, MAX * MAX * sizeof(double), cudaMemcpyHostToDevice);

    dim3 dimBlock(BLOCKS, BLOCKS);
    dim3 dimGrid(MAX / BLOCKS, MAX / BLOCKS);

    cudaEventCreate(&start); // 시간 기록
    cudaEventCreate(&stop);
    cudaEventRecord(start, 0);

    matrix_multiplication<<<dimGrid, dimBlock>>>(
        cuda_C, cuda_A, cuda_B); // matrix multiplication
    // cudaDeviceSynchronize();

    cudaEventRecord(stop, 0);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&time, start, stop);
    cudaEventDestroy(start);
    cudaEventDestroy(stop);

    cudaMemcpy(C, cuda_C, MAX * MAX * sizeof(double), cudaMemcpyDeviceToHost);

    if (t == 0) {
      for (int i = 0; i < MAX * MAX; i++) {
        if ((i + 1) % MAX == 0) {
          printf("%.1lf\n", C[i]);
        } else {
          printf("%.1lf ", C[i]);
        }
      }
    }

    time_avg += time;

    // 메모리 해제
    cudaFree(cuda_A);
    cudaFree(cuda_B);
    cudaFree(cuda_C);
  }

  printf("time : %f\n", time_avg / (float)EPOCH); // 평균 소요 시간 출력
  return 0;
}
