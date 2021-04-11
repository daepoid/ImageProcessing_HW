#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <windows.h>
#include <wingdi.h>
#define MAX 512
using namespace std;

typedef struct headers {
  BITMAPFILEHEADER hFile;
  BITMAPINFOHEADER hInfo;
  RGBQUAD hRGB[256];
} BITMAPHEADERS;

BYTE *rotate_raw_data(BYTE *image) {
  BYTE *newimage = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      newimage[(MAX - j - 1) * MAX + i] = image[i * MAX + j];
    }
  }
  return newimage;
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

int main() {
  BITMAPHEADERS bh;
  generate_headers(bh);

  BYTE *image;
  FILE *output_file, *input_file;
  input_file = fopen("outputs/mach_band_512x512.raw", "rb");
  image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  fread(image, sizeof(BYTE), MAX * MAX, input_file);
  fclose(input_file);

  image = rotate_raw_data(image);
  reverse_raw_data(image);

  output_file = fopen("outputs/new_rotated_mach_band_512x512.bmp", "wb");
  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);

  fwrite(image, sizeof(BYTE), MAX * MAX, output_file);
  fclose(output_file);
  free(image);
  printf("Success\n");
  return 0;
}
