#include <cstdio>
#include <iostream>
#include <math.h>
#define pi 3.1459265
using namespace std;
#define M 512
#define N 512 //파일 크기는 256*256
#define n 8
#define m 8

int main() {
  int matrix[m][n];
  unsigned char *inputimage;
  unsigned char *outputimage;
  double *DCTimage;
  unsigned char *header;
  FILE *InFile, *OutFile;
  header = new unsigned char[54 + 1024];  //헤더부분
  inputimage = new unsigned char[M * N];  //입력받을 값의 배열
  DCTimage = new double[M * N];           // DCT된 값을 저장할 배열
  outputimage = new unsigned char[M * N]; //출력할 값의 배열

  fopen_s(&InFile, "lena_bmp_512x512_new.bmp", "rb"); //입력 파일을 불러옵니다.
  if (InFile == NULL)
    cout << "파일 불러오기 실패";
  fread(header, sizeof(unsigned char), 54 + 1024, InFile);
  fread(inputimage, sizeof(unsigned char), M * N, InFile);

  //-------------------------------------------------------------- DCT
  for (int s = 0; s < M * N / (m * n); s++) {
    double dct1, ci, cj;
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        matrix[i][j] =
            inputimage[j + m * i + m * n * s]; // 8*8 단위로 8*8 크기의 matrix에
                                               // 입력받은 값을 집어넣습니다.
      }
    }
    for (int i = 0; i < m; i++) { //여기선 편의상 ppt자료 공식의 기호와 달리
                                  // i=u, j=v, i=k, j=l입니다.
      for (int j = 0; j < n; j++) {
        float sum = 0;
        if (i == 0)
          ci = 1 / sqrt(2);
        else
          ci = 1;
        if (j == 0)
          cj = 1 / sqrt(2);
        else
          cj = 1;
        for (int k = 0; k < m; k++) {
          for (int l = 0; l < n; l++) {
            dct1 =
                matrix[k][l] * cos((2 * k + 1) * i * pi / (2 * m)) *
                cos((2 * l + 1) * j * pi / (2 * n)); //공식 대로 계산해줍니다.
            sum += dct1;
          }
        }
        DCTimage[j + m * i + m * n * s] = (2 * ci * cj * sum / sqrt(m * n));
      }
    }
  }
  //---------------------------------------------------------------

  //-------------------------------------------------------------IDCT-------------------------
  for (int s = 0; s < M * N / (m * n); s++) {
    double dct1, ci, cj;
    for (int i = 0; i < m; i++) {
      for (int j = 0; j < n; j++) {
        matrix[i][j] =
            DCTimage[j + m * i + m * n * s]; // 8*8 단위로 8*8 크기의 matrix에
                                             // DCT된 값을 집어넣습니다.
      }
    }
    for (int i = 0; i < m;
         i++) { //편의상 ppt자료 공식의 기호와 달리 u=k v=l입니다.
      for (int j = 0; j < n; j++) {
        float sum = 0;

        for (int k = 0; k < m; k++) {
          for (int l = 0; l < n; l++) {
            if (k == 0)
              ci = 1 / sqrt(2);
            else
              ci = 1;
            if (l == 0)
              cj = 1 / sqrt(2);
            else
              cj = 1;
            dct1 =
                cj * ci / 4 * matrix[k][l] *
                cos((2 * i + 1) * k * pi / (2 * m)) *
                cos((2 * j + 1) * l * pi / (2 * n)); //공식 대로 계산해줍니다.
            sum += dct1;
          }
        }
        outputimage[j + m * i + m * n * s] = (int)(sum);
      }
    }
  }
  //-------------------------------------------------------------------------------------
  fopen_s(&OutFile, "outputs/lena_IDCT.bmp", "wb");
  if (OutFile == NULL) {
    cout << "Output image is not generated.";
    exit(-1);
  } else {
    cout << "- Process Completed -" << endl;
    cout << "Output image file name is 'lena_IDCT.bmp'" << endl;
  }

  fwrite(header, sizeof(unsigned char), 54 + 1024, OutFile);
  fwrite(outputimage, sizeof(unsigned char), M * N, OutFile);
  fclose(OutFile);
  fclose(InFile);

  delete[] header;
  delete[] inputimage;
  delete[] outputimage;
  system("pause");
}