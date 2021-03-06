#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <windows.h>
#include <wingdi.h>
#define MAX 512
#define B_SIZE 8
#define pi 3.141592653589793238
#define RADIAN 40
#define nint(x) ((x) < 0. ? (int)((x)-0.5) : (int)((x) + 0.5))
using namespace std;

typedef struct headers {
  BITMAPFILEHEADER hFile;
  BITMAPINFOHEADER hInfo;
  RGBQUAD hRGB[256];
} BITMAPHEADERS;

void generate_headers(BITMAPHEADERS &bh) {
  bh.hFile.bfType = 0x4D42;
  bh.hFile.bfReserved1 = 0;
  bh.hFile.bfReserved2 = 0;
  bh.hFile.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
                       sizeof(RGBQUAD) * 256;
  bh.hFile.bfSize = bh.hFile.bfOffBits + MAX * MAX;

  bh.hInfo.biSize = 40;
  bh.hInfo.biWidth = MAX;
  bh.hInfo.biHeight = MAX;
  bh.hInfo.biPlanes = 1;
  bh.hInfo.biBitCount = 8;
  bh.hInfo.biCompression = 0;
  bh.hInfo.biSizeImage = MAX * MAX;
  bh.hInfo.biXPelsPerMeter = 0;
  bh.hInfo.biYPelsPerMeter = 0;
  bh.hInfo.biClrUsed = 0;
  bh.hInfo.biClrImportant = 0;

  for (int i = 0; i < 256; i++) {
    bh.hRGB[i].rgbBlue = i;
    bh.hRGB[i].rgbGreen = i;
    bh.hRGB[i].rgbRed = i;
    bh.hRGB[i].rgbReserved = 0;
  }
}

void reverse_raw_data(BYTE *image) {
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      if (i < MAX / 2) {
        swap(image[i * MAX + j], image[(MAX - i - 1) * MAX + j]);
      }
    }
  }
}

void make_bmp(BYTE *output_image, string output_name) {
  reverse_raw_data(output_image);

  BITMAPHEADERS bh;
  generate_headers(bh);
  string PATH = "outputs/" + output_name + ".bmp";
  FILE *output_file = fopen(PATH.c_str(), "wb");

  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);

  fwrite(output_image, sizeof(BYTE), MAX * MAX, output_file);
  // fclose(output_file);
  return;
}

void LUT_based_DCT(double ix[][B_SIZE]) {
  double x[B_SIZE][B_SIZE], z[B_SIZE][B_SIZE], y[B_SIZE], yy[B_SIZE];
  double c[RADIAN], s[RADIAN], ft[4], fxy[4], zz;

  // cos, sin ??? ?????? ??????
  for (int i = 0; i < RADIAN; i++) {
    zz = pi * (double)(i + 1) / 64.0;
    c[i] = cos(zz);
    s[i] = sin(zz);
  }

  // x??? ix??? ????????? ????????? B_SIZE * B_SIZE ?????? ????????? ????????????.
  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      x[i][j] = (double)ix[i][j];
    }
  }

  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      y[j] = x[i][j];
    }

    for (int j = 0; j < 4; j++) {
      ft[j] = y[j] + y[7 - j];
    }

    fxy[0] = ft[0] + ft[3];
    fxy[1] = ft[1] + ft[2];
    fxy[2] = ft[1] - ft[2];
    fxy[3] = ft[0] - ft[3];

    ft[0] = c[15] * (fxy[0] + fxy[1]);
    ft[2] = c[15] * (fxy[0] - fxy[1]);
    ft[1] = s[7] * fxy[2] + c[7] * fxy[3];
    ft[3] = -s[23] * fxy[2] + c[23] * fxy[3];

    for (int j = 4; j < 8; j++) {
      yy[j] = y[7 - j] - y[j];
    }

    y[4] = yy[4];
    y[7] = yy[7];
    y[5] = c[15] * (-yy[5] + yy[6]);
    y[6] = c[15] * (yy[5] + yy[6]);

    yy[4] = y[4] + y[5];
    yy[5] = y[4] - y[5];
    yy[6] = -y[6] + y[7];
    yy[7] = y[6] + y[7];

    y[0] = ft[0];
    y[4] = ft[2];
    y[2] = ft[1];
    y[6] = ft[3];
    y[1] = s[3] * yy[4] + c[3] * yy[7];
    y[5] = s[19] * yy[5] + c[19] * yy[6];
    y[3] = -s[11] * yy[5] + c[11] * yy[6];
    y[7] = -s[27] * yy[4] + c[27] * yy[7];

    for (int j = 0; j < B_SIZE; j++) {
      z[i][j] = y[j];
    }
  }

  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      y[j] = z[j][i];
    }

    for (int j = 0; j < 4; j++) {
      ft[j] = y[j] + y[7 - j];
    }

    fxy[0] = ft[0] + ft[3];
    fxy[1] = ft[1] + ft[2];
    fxy[2] = ft[1] - ft[2];
    fxy[3] = ft[0] - ft[3];

    ft[0] = c[15] * (fxy[0] + fxy[1]);
    ft[2] = c[15] * (fxy[0] - fxy[1]);
    ft[1] = s[7] * fxy[2] + c[7] * fxy[3];
    ft[3] = -s[23] * fxy[2] + c[23] * fxy[3];

    for (int j = 4; j < 8; j++) {
      yy[j] = y[7 - j] - y[j];
    }

    y[4] = yy[4];
    y[7] = yy[7];
    y[5] = c[15] * (-yy[5] + yy[6]);
    y[6] = c[15] * (yy[5] + yy[6]);

    yy[4] = y[4] + y[5];
    yy[5] = y[4] - y[5];
    yy[6] = -y[6] + y[7];
    yy[7] = y[6] + y[7];

    y[0] = ft[0];
    y[4] = ft[2];
    y[2] = ft[1];
    y[6] = ft[3];
    y[1] = s[3] * yy[4] + c[3] * yy[7];
    y[5] = s[19] * yy[5] + c[19] * yy[6];
    y[3] = -s[11] * yy[5] + c[11] * yy[6];
    y[7] = -s[27] * yy[4] + c[27] * yy[7];

    for (int j = 0; j < B_SIZE; j++) {
      y[j] = y[j] / 4.0;
    }

    for (int j = 0; j < B_SIZE; j++) {
      z[j][i] = y[j];
    }
  }

  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      ix[i][j] = nint(z[i][j]);
    }
  }
}

void LUT_based_IDCT(double ix[][B_SIZE]) {
  double x[B_SIZE][B_SIZE], z[B_SIZE][B_SIZE], y[B_SIZE], yy[B_SIZE];
  double c[RADIAN], s[RADIAN], ait[4], aixy[4], zz;

  for (int i = 0; i < RADIAN; i++) {
    zz = pi * (double)(i + 1) / 64.0;
    c[i] = cos(zz);
    s[i] = sin(zz);
  }

  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      x[i][j] = (double)ix[i][j];
    }
  }

  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      y[j] = x[j][i];
    }

    ait[0] = y[0];
    ait[1] = y[2];
    ait[2] = y[4];
    ait[3] = y[6];

    aixy[0] = c[15] * (ait[0] + ait[2]);
    aixy[1] = c[15] * (ait[0] - ait[2]);
    aixy[2] = s[7] * ait[1] - s[23] * ait[3];
    aixy[3] = c[7] * ait[1] + c[23] * ait[3];

    ait[0] = aixy[0] + aixy[3];
    ait[1] = aixy[1] + aixy[2];
    ait[2] = aixy[1] - aixy[2];
    ait[3] = aixy[0] - aixy[3];

    yy[4] = s[3] * y[1] - s[27] * y[7];
    yy[5] = s[19] * y[5] - s[11] * y[3];
    yy[6] = c[19] * y[5] + c[11] * y[3];
    yy[7] = c[3] * y[1] + c[27] * y[7];

    y[4] = yy[4] + yy[5];
    y[5] = yy[4] - yy[5];
    y[6] = -yy[6] + yy[7];
    y[7] = yy[6] + yy[7];

    yy[4] = y[4];
    yy[7] = y[7];
    yy[5] = c[15] * (-y[5] + y[6]);
    yy[6] = c[15] * (y[5] + y[6]);

    for (int j = 0; j < 4; j++) {
      y[j] = ait[j] + yy[7 - j];
    }

    for (int j = 4; j < 8; j++) {
      y[j] = ait[7 - j] - yy[j];
    }

    for (int j = 0; j < B_SIZE; j++) {
      z[j][i] = y[j];
    }
  }

  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      y[j] = z[i][j];
    }

    ait[0] = y[0];
    ait[1] = y[2];
    ait[2] = y[4];
    ait[3] = y[6];

    aixy[0] = c[15] * (ait[0] + ait[2]);
    aixy[1] = c[15] * (ait[0] - ait[2]);
    aixy[2] = s[7] * ait[1] - s[23] * ait[3];
    aixy[3] = c[7] * ait[1] + c[23] * ait[3];

    ait[0] = aixy[0] + aixy[3];
    ait[1] = aixy[1] + aixy[2];
    ait[2] = aixy[1] - aixy[2];
    ait[3] = aixy[0] - aixy[3];

    yy[4] = s[3] * y[1] - s[27] * y[7];
    yy[5] = s[19] * y[5] - s[11] * y[3];
    yy[6] = c[19] * y[5] + c[11] * y[3];
    yy[7] = c[3] * y[1] + c[27] * y[7];

    y[4] = yy[4] + yy[5];
    y[5] = yy[4] - yy[5];
    y[6] = -yy[6] + yy[7];
    y[7] = yy[6] + yy[7];

    yy[4] = y[4];
    yy[7] = y[7];
    yy[5] = c[15] * (-y[5] + y[6]);
    yy[6] = c[15] * (y[5] + y[6]);

    for (int j = 0; j < 4; j++) {
      y[j] = ait[j] + yy[7 - j];
    }

    for (int j = 4; j < 8; j++) {
      y[j] = ait[7 - j] - yy[j];
    }

    for (int j = 0; j < B_SIZE; j++) {
      z[i][j] = y[j] / 4.0;
    }
  }

  for (int i = 0; i < B_SIZE; i++) {
    for (int j = 0; j < B_SIZE; j++) {
      // ix[i][j] = ceil(z[i][j]);
      ix[i][j] = nint(z[i][j]);
    }
  }
}

int main() {
  FILE *input_file = fopen("BOAT512.raw", "rb");
  if (input_file == NULL) {
    printf("FILE ERROR\n");
    return 1;
  }

  BYTE *image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  double *transformed_image = (double *)malloc(sizeof(double) * MAX * MAX);
  BYTE *DCT_image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  BYTE *restored_image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);

  size_t n_size = fread(image, sizeof(BYTE), MAX * MAX, input_file);
  fclose(input_file);

  for (int i = 0; i < MAX; i += B_SIZE) {
    for (int j = 0; j < MAX; j += B_SIZE) {
      double copied[B_SIZE][B_SIZE];
      for (int a = i; a < i + B_SIZE; a++) {
        for (int b = j; b < j + B_SIZE; b++) {
          copied[a - i][b - j] = image[MAX * a + b];
        }
      }
      LUT_based_DCT(copied);
      double max_val = -2048.0;
      double min_val = 2048.0;
      for (int a = i; a < i + B_SIZE; a++) {
        for (int b = j; b < j + B_SIZE; b++) {
          transformed_image[MAX * a + b] = copied[a - i][b - j];
          max_val = max(max_val, copied[a - i][b - j]);
          min_val = min(min_val, copied[a - i][b - j]);
        }
      }

      for (int a = i; a < i + B_SIZE; a++) {
        for (int b = j; b < j + B_SIZE; b++) {
          double temp = (copied[a - i][b - j] - min_val) * 255;
          temp /= (max_val - min_val);
          if (temp > 255) {
            temp = 255;
          } else if (temp < 0) {
            temp = 0;
          }
          DCT_image[MAX * a + b] = temp;
        }
      }
    }
  }

  for (int i = 0; i < MAX; i += B_SIZE) {
    for (int j = 0; j < MAX; j += B_SIZE) {
      double copied[B_SIZE][B_SIZE];
      for (int a = i; a < i + B_SIZE; a++) {
        for (int b = j; b < j + B_SIZE; b++) {
          copied[a - i][b - j] = transformed_image[MAX * a + b];
        }
      }
      LUT_based_IDCT(copied);
      for (int a = i; a < i + B_SIZE; a++) {
        for (int b = j; b < j + B_SIZE; b++) {
          restored_image[MAX * a + b] = ceil(copied[a - i][b - j]);
        }
      }
    }
  }

  make_bmp(DCT_image, "DCT_image_BOAT512_CPP");
  make_bmp(restored_image, "Restored_DCT_BOAT512_CPP");

  // MSE?????? ?????????.
  long long int nTmp = 0;
  double dmse = 0;
  for (int i = 0; i < MAX * MAX; i++) {
    nTmp += (image[i] - restored_image[i]) * (image[i] - restored_image[i]);
  }

  dmse = (double)nTmp / (MAX * MAX);
  printf("MSE ??? : %lf\n", dmse);

  return 0;
}