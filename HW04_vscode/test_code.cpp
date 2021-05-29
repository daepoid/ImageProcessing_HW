#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 8
#define M_PIl 3.141592653589793238462643383279502884L /* pi */
#define PARAMETER 30

double getCval(int x, int y) {
  if (x == 0 && y == 0)
    return double(1) / double(N);
  else if (x != 0 && y != 0)
    return double(2) / double(N);
  else
    return sqrt(double(2)) / double(N);
}

int main() {
  double dTestArray[N][N] = {
      0,
  };
  double dDctArray[N][N] = {
      0,
  };
  double dRdctArray[N][N] = {
      0,
  };

  FILE *pOriginalFile = NULL;

  // 비교할 두 파일을 binary 모드로 연다.
  fopen_s(&pOriginalFile, "./PeopleOnStreet_1280x720_30_Original.yuv", "rb");

  if (pOriginalFile == NULL) {
    fputs("File error", stderr);
    exit(1);
  }

  // 두 파일에 대해 각각 resolution 크기만큼 할당 받는다.
  int resoulution_size = 1280 * 720;
  int nFrameSize = resoulution_size + resoulution_size / 2;
  unsigned char *read_data_origin = new unsigned char[nFrameSize];
  int *transformed_data = new int[nFrameSize];
  unsigned char *restored_data = new unsigned char[nFrameSize];
  memset(read_data_origin, 0, nFrameSize);
  memset(transformed_data, 0, nFrameSize * sizeof(int));
  memset(restored_data, 0, nFrameSize);
  size_t n_size = 0;

  // 한 프레임만 추출
  n_size =
      fread(read_data_origin, sizeof(unsigned char), nFrameSize, pOriginalFile);
  fclose(pOriginalFile);

  // 프레임 픽셀에 8x8 블럭 적용, dct 변환하여 저장
  int nIndex = 0;
  while (nIndex < nFrameSize) {
    int nX = (nIndex / N) % N;
    int nY = nIndex % N;
    dTestArray[nX][nY] = double(read_data_origin[nIndex]);
    nIndex++;

    if (nIndex % (N * N) != 0)
      continue;

    for (int x = 0; x < N; x++) {
      for (int y = 0; y < N; y++) {
        double dInputSum = 0;
        for (int i = 0; i < N; i++) {
          for (int j = 0; j < N; j++) {
            dInputSum += dTestArray[i][j] *
                         cos(((2 * double(j) + 1) * double(y) * M_PIl) /
                             (double(2) * double(N))) *
                         cos(((2 * double(i) + 1) * double(x) * M_PIl) /
                             (double(2) * double(N)));
          }
        }
        dDctArray[x][y] = (getCval(x, y) * dInputSum);
      }
    }

    for (int i = nIndex - (N * N); i < nIndex; i++) {
      nX = (i / N) % N;
      nY = i % N;
      transformed_data[i] =
          (int)((dDctArray[nX][nY]) / double(PARAMETER)); // 양자화
    }
  }

  // 역 dct
  nIndex = 0;
  while (nIndex < nFrameSize) {
    int nX = (nIndex / N) % N;
    int nY = nIndex % N;
    dDctArray[nX][nY] = double(transformed_data[nIndex]) *
                        double(PARAMETER); // 양자화한 만큼 다시 곱함
    nIndex++;

    if (nIndex % (N * N) != 0)
      continue;

    for (int i = 0; i < N; i++) {
      for (int j = 0; j < N; j++) {
        double dInputSum = 0;
        for (int x = 0; x < N; x++) {
          for (int y = 0; y < N; y++) {
            dInputSum += getCval(x, y) * dDctArray[x][y] *
                         cos(((2 * double(j) + 1) * double(y) * M_PIl) /
                             (double(2) * double(N))) *
                         cos(((2 * double(i) + 1) * double(x) * M_PIl) /
                             (double(2) * double(N)));
          }
        }
        dRdctArray[i][j] = dInputSum;
      }
    }

    for (int i = nIndex - (N * N); i < nIndex; i++) {
      nX = (i / N) % N;
      nY = i % N;
      restored_data[i] = (unsigned char)(dRdctArray[nX][nY]);
    }
  }

  // 검증용 yuv 파일
  FILE *pWriteFile = NULL;
  fopen_s(&pWriteFile, "./PeopleOnStreet_Reverse_DCT.yuv", "wb");
  n_size =
      fwrite(restored_data, sizeof(unsigned char), nFrameSize, pOriginalFile);

  if (pWriteFile != NULL)
    fclose(pWriteFile);

  // RMSE값을 구한다.
  long long nTmp = 0;
  double dmse = 0;
  for (int i = 0; i < nFrameSize; i++)
    nTmp += (read_data_origin[i] - restored_data[i]) *
            (read_data_origin[i] - restored_data[i]);

  dmse = (double)nTmp / nFrameSize;
  printf("MSE 값 : %f\n", dmse);

  delete[] read_data_origin;
  delete[] transformed_data;
  delete[] restored_data;

  getchar();

  return 0;
}