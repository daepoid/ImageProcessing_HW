#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <wingdi.h>
#define MAX 512
#define SNR 8.0
#define THRESHOLD 170
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
  /*
  RAW이미지와 달리 BMP에는 이미지가 뒤집혀 기록되어있다.
  따라서, BMP 파일을 RAW 파일과 같은 이미지로 보기 위해서는
  이미지의 위 아래의 값을 바꾸어 주어야 한다.
  */
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      if (i < MAX / 2) {
        swap(image[i * MAX + j], image[(MAX - i - 1) * MAX + j]);
      }
    }
  }
}

double Calculate_Variance(BYTE *image) {
  double avg_1 = 0, avg_2 = 0;
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      int temp = image[i * MAX + j];
      avg_1 += temp;
      avg_2 += temp * temp;
    }
  }

  avg_1 /= (MAX * MAX);
  avg_1 = pow(avg_1, 2);
  avg_2 /= (MAX * MAX);

  return avg_2 - avg_1;
}

int Gaussian(double sigma) {
  static int ready = 0;
  static float gstore;
  float v1, v2, r, fac, gaus;
  int r1, r2;
  if (ready == 0) {
    do {
      r1 = rand();
      r2 = rand();
      v1 = 2. * ((float)r1 / (float)RAND_MAX - 0.5);
      v2 = 2. * ((float)r2 / (float)RAND_MAX - 0.5);
      r = v1 * v1 + v2 * v2;
    } while (r > 1.0);
    fac = (float)sqrt((double)(-2 * log(r) / r));
    gstore = v1 * fac;
    gaus = v2 * fac;
    ready = 1;
  } else {
    ready = 0;
    gaus = gstore;
  }
  return gaus * sigma;
}

void Add_Gaussian_Noise(BYTE *image, BYTE *noise, double sigma) {
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      int s = image[i * MAX + j] + (Gaussian(sigma) + 0.5);
      if (s > 255) {
        s = 255;
      }
      if (s < 0) {
        s = 0;
      }
      noise[i * MAX + j] = s;
    }
  }
}

bool isvalid(int newy, int newx) {
  return 0 <= newy && newy < MAX && 0 <= newx && newx < MAX;
}

void masking(BYTE *noise, BYTE *image, int y, int x, int mask[][3][3]) {
  int avg_1 = 0, avg_2 = 0;

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      int newy = y + i - 1;
      int newx = x + j - 1;

      if (!isvalid(newy, newx)) {
        if (newy < 0) {
          newy += 1;
        } else if (newy >= MAX) {
          newy -= 1;
        }

        if (newx < 0) {
          newx += 1;
        } else if (newx >= MAX) {
          newx -= 1;
        }

        avg_1 += image[newy * MAX + newx] * mask[0][i][j];
        avg_2 += image[newy * MAX + newx] * mask[1][i][j];
      } else {
        avg_1 += image[newy * MAX + newx] * mask[0][i][j];
        avg_2 += image[newy * MAX + newx] * mask[1][i][j];
      }
    }
  }

  double val = sqrt(avg_1 * avg_1 + avg_2 * avg_2);

  if (THRESHOLD <= val) {
    noise[y * MAX + x] = (BYTE)255;
  } else {
    noise[y * MAX + x] = (BYTE)0;
  }
}

void make_bmp(BYTE *output_image, string output_name) {
  reverse_raw_data(output_image);

  BITMAPHEADERS bh;
  generate_headers(bh);
  string PATH = "outputs/" + output_name + "_EdgeDetection.bmp";
  FILE *output_file = fopen(PATH.c_str(), "wb");
  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);

  fwrite(output_image, sizeof(BYTE), MAX * MAX, output_file);
  fclose(output_file);
  return;
}

void EdgeDetection_3(BYTE *image, int mask[][3][3], string output_name) {
  BYTE *noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  // memcpy(noise, image, sizeof(image));

  BYTE *ori_image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  // memcpy(ori_image, image, sizeof(image));

  // Original Image Edge Detection
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      masking(ori_image, image, i, j, mask);
    }
  }

  // Add Gaussian Noise
  double variance = Calculate_Variance(image);
  double stddev_noise = sqrt(variance / pow(10.0, ((double)SNR / 10)));
  Add_Gaussian_Noise(image, noise, stddev_noise);

  // Noisy Image Edge Detection
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      masking(noise, image, i, j, mask);
    }
  }
  double sum = 0;

  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      sum = sum + abs(ori_image[i * MAX + j] - noise[i * MAX + j]);
    }
  }
  printf("%lf\n", sum);

  make_bmp(ori_image, output_name + "_original");
  make_bmp(noise, output_name);
}

void Stochastic_EdgeDetection(BYTE *image) {
  double Stochastic[5][5] = {{0.267, 0.364, 0, -0.364, -0.267},
                             {0.373, 0.562, 0, -0.562, -0.373},
                             {0.463, 1.000, 0, -1.000, -0.463},
                             {0.373, 0.562, 0, -0.562, -0.373},
                             {0.267, 0.364, 0, -0.364, -0.267}};
  BYTE *noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  memcpy(noise, image, sizeof(image));

  double variance = Calculate_Variance(image);
  double stddev_noise = sqrt(variance / pow(10.0, ((double)SNR / 10)));
  Add_Gaussian_Noise(image, noise, stddev_noise);
  make_bmp(noise, "Stochastic");
}

int main() {
  BYTE *image;
  FILE *input_file;
  input_file = fopen("lena_raw_512x512.raw", "rb");
  image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  fread(image, sizeof(BYTE), MAX * MAX, input_file);
  fclose(input_file);

  int Roberts[2][3][3] = {{{0, 0, -1}, {0, 1, 0}, {0, 0, 0}},
                          {{-1, 0, 0}, {0, 1, 0}, {0, 0, 0}}};

  int Prewitt[2][3][3] = {{{1, 0, -1}, {1, 0, -1}, {1, 0, -1}},
                          {{-1, -1, -1}, {0, 0, 0}, {1, 1, 1}}};

  int Sobel[2][3][3] = {{{1, 0, -1}, {2, 0, -2}, {1, 0, -1}},
                        {{-1, -2, -1}, {0, 0, 0}, {1, 2, 1}}};

  EdgeDetection_3(image, Roberts, "Roberts");
  EdgeDetection_3(image, Sobel, "Sobel");
  EdgeDetection_3(image, Prewitt, "Prewitt");
  Stochastic_EdgeDetection(image);
  return 0;
}