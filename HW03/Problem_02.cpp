#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <wingdi.h>
#define MAX 512
#define SNR 9.0
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
  string PATH = "outputs/" + output_name + "_EdgeDetection.bmp";
  FILE *output_file = fopen(PATH.c_str(), "wb");

  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);

  fwrite(output_image, sizeof(BYTE), MAX * MAX, output_file);
  // fclose(output_file);
  return;
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

void Add_Gaussian_Noise(BYTE *image, BYTE *noise, double sigma) {
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

int calculate_MSE(BYTE *masked_image, BYTE *masked_noise) {
  long long int mse = 0;
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      mse += pow((masked_image[i * MAX + j] - masked_noise[i * MAX + j]), 2);
    }
  }
  return (int)(mse / (MAX * MAX));
}

void Lowpass_Masking(BYTE *masked_image, BYTE *image, double mask[9]) {
  int dy[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
  int dx[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

  for (int y = 0; y < MAX; y++) {
    for (int x = 0; x < MAX; x++) {
      double avg_1 = 0.0;
      double avg_2 = 0.0;

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
        avg_1 += image[newy * MAX + newx] * mask[i];
      }
      masked_image[y * MAX + x] = (BYTE)(avg_1 + 0.5);
    }
  }
}

void Lowpass_EdgeDetection(BYTE *image, BYTE *noise, string output_name) {
  double Lowpass[9] = {1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0,
                       1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0, 1.0 / 9.0};

  // Original Image Edge Detection
  BYTE *masked_original = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  Lowpass_Masking(masked_original, image, Lowpass);

  // Noisy Image Edge Detection
  BYTE *masked_noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  Lowpass_Masking(masked_noise, noise, Lowpass);
  printf("%d\n", calculate_MSE(masked_original, masked_noise));

  make_bmp(masked_original, output_name + "_Original");
  make_bmp(masked_noise, output_name);

  free(masked_noise);
  free(masked_original);
}

void Median_Masking(BYTE *masked, BYTE *image) {
  int dy[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
  int dx[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};

  for (int y = 0; y < MAX; y++) {
    for (int x = 0; x < MAX; x++) {
      double arr[9] = {};

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
        arr[i] = image[newy * MAX + newx];
      }
      sort(arr, arr + 9);
      masked[y * MAX + x] = (BYTE)(arr[4] + 0.5);
    }
  }
}

void EdgeDetection_median(BYTE *image, BYTE *noise, string output_name) {
  BYTE *masked_original = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  Median_Masking(masked_original, image);

  // Noisy Image Edge Detection
  BYTE *masked_noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  Median_Masking(masked_noise, noise);

  printf("%d\n", calculate_MSE(masked_original, masked_noise));

  make_bmp(masked_original, output_name + "_Original");
  make_bmp(masked_noise, output_name);

  free(masked_noise);
  free(masked_original);
}

int main() {
  FILE *input_file = fopen("BOAT512.raw", "rb");
  if (input_file == NULL) {
    printf("FILE ERROR\n");
    return 0;
  }

  BYTE *image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  fread(image, sizeof(BYTE), MAX * MAX, input_file);
  fclose(input_file);

  // Add Gaussian Noise
  BYTE *noise = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  double variance = Calculate_Variance(image);
  double stddev_noise = sqrt(variance / pow(10.0, ((double)SNR / 10)));
  Add_Gaussian_Noise(image, noise, stddev_noise);

  Lowpass_EdgeDetection(image, noise, "2-1. Lowpass");
  EdgeDetection_median(image, noise, "2-2. Median");

  return 0;
}