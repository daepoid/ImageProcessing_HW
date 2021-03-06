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
#define SNR 8.0
#define WHITE 255
#define BLACK 0
#define THRESHOLD 150
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

double calculate_variance(BYTE *image) {
  double avg_1 = 0, avg_2 = 0;

  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      int temp = image[i * MAX + j];
      avg_1 += temp;
      avg_2 += temp * temp;
    }
  }

  avg_1 /= (double)(MAX * MAX);
  avg_2 /= (double)(MAX * MAX);

  return avg_2 - (avg_1 * avg_1);
}

double Gaussian(double sigma) {
  static int ready = 0;
  static double gstore;
  double v1, v2, r, fac, gaus;
  int r1, r2;

  if (ready == 0) {
    do {
      r1 = rand();
      r2 = rand();
      v1 = 2. * ((double)r1 / (double)RAND_MAX - 0.5);
      v2 = 2. * ((double)r2 / (double)RAND_MAX - 0.5);
      r = v1 * v1 + v2 * v2;
    } while (r > 1.0);
    fac = (double)sqrt((double)(-2 * log(r) / r));
    gstore = v1 * fac;
    gaus = v2 * fac;
    ready = 1;
  } else {
    ready = 0;
    gaus = gstore;
  }
  return gaus * sigma;
}

void add_gaussian_noise(BYTE *image, BYTE *noise, double sigma) {
  int s;
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      s = (int)image[i * MAX + j] + (Gaussian(sigma) + 0.5);
      if (s > 255) {
        s = 255;
      }
      if (s < 0) {
        s = 0;
      }
      noise[i * MAX + j] = (BYTE)s;
    }
  }
}

void masking(BYTE *masked, BYTE *image, int mask[][9]) {
  int dy[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
  int dx[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

  for (int y = 0; y < MAX; y++) {
    for (int x = 0; x < MAX; x++) {
      int avg_1 = 0;
      int avg_2 = 0;

      for (int i = 0; i < 9; i++) {
        int newy = y + dy[i];
        int newx = x + dx[i];

        if (newy < 0) {
          newy = 0;
        } else if (newy >= MAX) {
          newy = MAX - 1;
        }

        if (newx < 0) {
          newx = 0;
        } else if (newx >= MAX) {
          newx = MAX - 1;
        }

        avg_1 += image[newy * MAX + newx] * mask[0][i];
        avg_2 += image[newy * MAX + newx] * mask[1][i];
      }

      // int val = sqrt(avg_1 * avg_1 + avg_2 * avg_2);
      //   double val = abs(avg_1 + 0.5) + abs(avg_2 + 0.5);
      //   double val = abs(avg_1) + abs(avg_2);
      int val = abs(avg_1) + abs(avg_2);
      if (THRESHOLD <= val) {
        masked[y * MAX + x] = (BYTE)WHITE;
      } else {
        masked[y * MAX + x] = (BYTE)BLACK;
      }
    }
  }
}

void masking(BYTE *masked_image, BYTE *image, double mask[][25]) {
  int dy[25] = {-2, -2, -2, -2, -2, -1, -1, -1, -1, -1, 0, 0, 0,
                0,  0,  1,  1,  1,  1,  1,  2,  2,  2,  2, 2};
  int dx[25] = {-2, -1, 0,  1,  2, -2, -1, 0,  1,  2, -2, -1, 0,
                1,  2,  -2, -1, 0, 1,  2,  -2, -1, 0, 1,  2};

  for (int y = 0; y < MAX; y++) {
    for (int x = 0; x < MAX; x++) {
      double avg_1 = 0.0;
      double avg_2 = 0.0;

      for (int i = 0; i < 25; i++) {
        int newy = y + dy[i];
        int newx = x + dx[i];

        if (newy < 0) {
          newy = 0;
        } else if (newy >= MAX) {
          newy = MAX - 1;
        }

        if (newx < 0) {
          newx = 0;
        } else if (newx >= MAX) {
          newx = MAX - 1;
        }
        avg_1 += (double)image[newy * MAX + newx] * mask[0][i];
        avg_2 += (double)image[newy * MAX + newx] * mask[1][i];
      }

      //   double val = sqrt(avg_1 * avg_1 + avg_2 * avg_2);
      //   double val = abs(avg_1 + 0.5) + abs(avg_2 + 0.5);
      double val = abs(avg_1) + abs(avg_2);
      if (THRESHOLD <= val) {
        masked_image[y * MAX + x] = (BYTE)WHITE;
      } else {
        masked_image[y * MAX + x] = (BYTE)BLACK;
      }
    }
  }
}

double calculate_error_rate(BYTE *masked_original, BYTE *masked_noise) {
  double n0 = 0, n1 = 0;
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      if (masked_original[i * MAX + j] == WHITE) {
        n0++;
      }

      if (masked_original[i * MAX + j] != masked_noise[i * MAX + j]) {
        n1++;
      }
    }
  }
  return n1 / n0;
}

void edge_detection(BYTE *image, BYTE *noise, int mask[][9],
                    string output_name) {
  // Original Image Edge Detection
  BYTE *masked_original = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  masking(masked_original, image, mask);

  // Noisy Image Edge Detection
  BYTE *masked_noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  masking(masked_noise, noise, mask);

  printf("%s Edge Detection Error Rate : %lf\n", output_name.c_str(),
         calculate_error_rate(masked_original, masked_noise));

  make_bmp(masked_original, output_name + "_Original");
  make_bmp(masked_noise, output_name);

  free(masked_noise);
  free(masked_original);
}

void edge_detection(BYTE *image, BYTE *noise, double mask[][25],
                    string output_name) {

  // Original Image Edge Detection
  BYTE *masked_original = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  masking(masked_original, image, mask);

  // Noisy Image Edge Detection
  BYTE *masked_noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  masking(masked_noise, noise, mask);

  printf("%s Edge Detection Error Rate : %lf\n", output_name.c_str(),
         calculate_error_rate(masked_original, masked_noise));

  make_bmp(masked_original, output_name + "_Original");
  make_bmp(masked_noise, output_name);
}

int main() {
  srand((unsigned int)time(NULL));
  int Roberts[2][9] = {{0, 0, -1, 0, 1, 0, 0, 0, 0},
                       {-1, 0, 0, 0, 1, 0, 0, 0, 0}};
  int Prewitt[2][9] = {{1, 0, -1, 1, 0, -1, 1, 0, -1},
                       {-1, -1, -1, 0, 0, 0, 1, 1, 1}};
  int Sobel[2][9] = {{1, 0, -1, 2, 0, -2, 1, 0, -1},
                     {-1, -2, -1, 0, 0, 0, 1, 2, 1}};
  double Stochastic[2][25] = {
      {0.267,  0.364,  0,     -0.364, -0.267, 0.373,  0.562, 0,     -0.562,
       -0.373, 0.463,  1.000, 0,      -1.000, -0.463, 0.373, 0.562, 0,
       -0.562, -0.373, 0.267, 0.364,  0,      -0.364, -0.267},
      {-0.267, -0.373, -0.463, -0.373, -0.267, -0.364, -0.562, -1.000, -0.562,
       -0.364, 0,      0,      0,      0,      0,      0.364,  0.562,  1.000,
       0.562,  0.364,  0.267,  0.373,  0.463,  0.373,  0.267}};

  FILE *input_file = fopen("lena_raw_512x512.raw", "rb");
  if (input_file == NULL) {
    printf("FILE ERROR\n");
    return 0;
  }

  BYTE *image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  fread(image, sizeof(BYTE), MAX * MAX, input_file);
  fclose(input_file);

  // Add Gaussian Noise
  BYTE *noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  double variance = calculate_variance(image);
  double stddev_noise = sqrt(variance / pow(10.0, ((double)SNR / 10)));
  add_gaussian_noise(image, noise, stddev_noise);

  make_bmp(image, "0-1. Lena_Original");
  reverse_raw_data(image);
  make_bmp(noise, "0-2. Lena_GaussianNoise");
  reverse_raw_data(noise);

  edge_detection(image, noise, Roberts, "1-1. Roberts");
  edge_detection(image, noise, Prewitt, "1-2. Prewitt");
  edge_detection(image, noise, Sobel, "1-3. Sobel");

  edge_detection(image, noise, Stochastic, "1-4. Stochastic");

  return 0;
}