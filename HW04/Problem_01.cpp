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
#define B_size 8
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
  string PATH = "outputs/" + output_name + "_EdgeDetection(" +
                to_string(THRESHOLD) + ").bmp";
  FILE *output_file = fopen(PATH.c_str(), "wb");

  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);

  fwrite(output_image, sizeof(BYTE), MAX * MAX, output_file);
  // fclose(output_file);
  return;
}

void dct_block(int ix[][B_size]) {
  static float pi = 3.141592653589793238;
  float x[B_size][B_size], z[B_size][B_size], y[B_size], c[40], s[40], ft[4],
      fxy[4], yy[B_size], zz;

  for (int i = 0; i < 40; i++) {
    zz = pi * (float)(i + 1) / 64.0;
    c[i] = cos(zz);
    s[i] = sin(zz);
  }

  for (int i = 0; i < B_size; i++) {
    for (int j = 0; j < B_size; j++) {
      x[i][j] = (float)ix[i][j];
    }
  }

  for (int i = 0; i < B_size; i++) {
    for (int j = 0; j < B_size; j++) {
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

    for (int j = 0; j < B_size; j++) {
      z[i][j] = y[j];
    }
  }

  for (int i = 0; i < B_size; i++) {
    for (int j = 0; j < B_size; j++) {
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

    for (int j = 0; j < B_size; j++) {
      y[j] = y[j] / 4.0;
    }

    for (int j = 0; j < B_size; j++) {
      z[j][i] = y[j];
    }
  }

  for (int i = 0; i < B_size; i++) {
    for (int j = 0; j < B_size; j++) {
      ix[i][j] = nint(z[i][j]);
    }
  }
}

int main() {
  FILE *input_file = fopen("lena_raw_512x512.raw", "rb");
  if (input_file == NULL) {
    printf("FILE ERROR\n");
    return 0;
  }

  BYTE *image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  fread(image, sizeof(BYTE), MAX * MAX, input_file);
  fclose(input_file);

  return 0;
}