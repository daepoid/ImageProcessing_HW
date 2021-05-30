#include "opencv2/core/types_c.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/opencv.hpp"
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
#pragma warning(disable : 4996)
#define MAX 512
#define B_SIZE 8
#define pi 3.141592653589793238
using namespace std;
using namespace cv;

typedef struct headers {
  BITMAPFILEHEADER hFile;
  BITMAPINFOHEADER hInfo;
  RGBQUAD hRGB[256];
} BITMAPHEADERS;

void generate_headers(BITMAPHEADERS& bh) {
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

void reverse_raw_data(BYTE* image) {
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      if (i < MAX / 2) {
        swap(image[i * MAX + j], image[(MAX - i - 1) * MAX + j]);
      }
    }
  }
}

void make_bmp(BYTE* output_image, string output_name) {
  reverse_raw_data(output_image);

  BITMAPHEADERS bh;
  generate_headers(bh);
  string PATH = "outputs/" + output_name + ".bmp";
  FILE* output_file = fopen(PATH.c_str(), "wb");

  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);

  fwrite(output_image, sizeof(BYTE), MAX * MAX, output_file);
  // fclose(output_file);
  return;
}

void DCT(BYTE* input_image, double* DCT_image) {
  int matrix[B_SIZE][B_SIZE];
  for (int s = 0; s < MAX * MAX / (B_SIZE * B_SIZE); s++) {
    double dct_val, ci, cj;
    for (int i = 0; i < B_SIZE; i++) {
      for (int j = 0; j < B_SIZE; j++) {
        // B_SIZE * B_SIZE ũ���� matrix�� ���� ������ �ִ´�.
        // B_SIZE * B_SIZE * s�� MAX * MAX ������ �迭����
        // ���ϴ� ���� ã�� ���� ��ġ ������
        matrix[i][j] = input_image[j + B_SIZE * i + B_SIZE * B_SIZE * s];
      }
    }

    for (int i = 0; i < B_SIZE; i++) {
      for (int j = 0; j < B_SIZE; j++) {
        double sum = 0;
        if (i == 0) {
          ci = 1 / sqrt(2);
        }
        else {
          ci = 1;
        }

        if (j == 0) {
          cj = 1 / sqrt(2);
        }
        else {
          cj = 1;
        }

        for (int k = 0; k < B_SIZE; k++) {
          for (int l = 0; l < B_SIZE; l++) {
            // DCT ���� ���
            dct_val = matrix[k][l] * cos((2 * k + 1) * i * pi / (2 * B_SIZE)) *
              cos((2 * l + 1) * j * pi / (2 * B_SIZE));
            sum += dct_val;
          }
        }
        DCT_image[j + B_SIZE * i + B_SIZE * B_SIZE * s] =
          (2 * ci * cj * sum / sqrt(B_SIZE * B_SIZE));
      }
    }
  }
}

void IDCT(double* DCT_image, BYTE* output_image) {
  int matrix[B_SIZE][B_SIZE];
  for (int s = 0; s < MAX * MAX / (B_SIZE * B_SIZE); s++) {
    double dct_val, ci, cj;
    for (int i = 0; i < B_SIZE; i++) {
      for (int j = 0; j < B_SIZE; j++) {
        // B_SIZE * B_SIZE ũ���� matrix�� ���� ������ �ִ´�.
        // B_SIZE * B_SIZE * s�� MAX * MAX ������ �迭����
        // ���ϴ� ���� ã�� ���� ��ġ ������
        matrix[i][j] = DCT_image[j + B_SIZE * i + B_SIZE * B_SIZE * s];
      }
    }
    for (int i = 0; i < B_SIZE; i++) {
      for (int j = 0; j < B_SIZE; j++) {
        double sum = 0;
        for (int k = 0; k < B_SIZE; k++) {
          for (int l = 0; l < B_SIZE; l++) {
            if (k == 0) {
              ci = 1 / sqrt(2);
            }
            else {
              ci = 1;
            }
            if (l == 0) {
              cj = 1 / sqrt(2);
            }
            else {
              cj = 1;
            }
            dct_val = cj * ci / 4 * matrix[k][l] *
              cos((2 * i + 1) * k * pi / (2 * B_SIZE)) *
              cos((2 * j + 1) * l * pi / (2 * B_SIZE));
            sum += dct_val;
          }
        }
        output_image[j + B_SIZE * i + B_SIZE * B_SIZE * s] = (BYTE)(sum);
      }
    }
  }
}

void plot_frequency_spectrum(double* DCT_image) {
  double* temp_image = (double*)malloc(sizeof(double) * MAX * MAX);
  memcpy(temp_image, DCT_image, sizeof(double) * MAX * MAX);

  MatND hist;
  const int* channel_numbers = { 0 };
  float channel_range[] = { 0.0, 255.0 };
  const float* channel_ranges = channel_range;
  int number_bins = 255;


  double max_val = -1;
  double min_val = 999;
  for (int i = 0; i < MAX * MAX; i++) {
    min_val = min(min_val, temp_image[i]);
    max_val = max(max_val, temp_image[i]);
  }
  printf("%lf\n%lf\n", min_val, max_val);
}

int main() {
  FILE* input_file = fopen("lena_raw_512x512.raw", "rb");
  if (input_file == NULL) {
    printf("FILE ERROR\n");
    return 1;
  }

  int frame_size = MAX * MAX;
  BYTE* image = (BYTE*)malloc(sizeof(BYTE) * frame_size);
  double* transformed_image = (double*)malloc(sizeof(double) * frame_size);
  BYTE* restored_image = (BYTE*)malloc(sizeof(BYTE) * frame_size);

  size_t n_size = fread(image, sizeof(BYTE), frame_size, input_file);
  fclose(input_file);
  // Perform 8x8 forward DCT
  DCT(image, transformed_image);
  // plot the frequency spectrum on the monitor in proper scale for easy
  // observation.
  plot_frequency_spectrum(transformed_image);
  // Perform 8x8 inverse DCT
  IDCT(transformed_image, restored_image);
  // restore bmp file
  make_bmp(restored_image, "Restored_DCT_Lena");

  // compute MSE
  long long int lli_temp = 0;
  double mse = 0;
  for (int i = 0; i < frame_size; i++) {
    lli_temp += (image[i] - restored_image[i]) * (image[i] - restored_image[i]);
  }

  mse = (double)lli_temp / frame_size;
  printf("MSE �� : %f\n", mse);

  return 0;
}
