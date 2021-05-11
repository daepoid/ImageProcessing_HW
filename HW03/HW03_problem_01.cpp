#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <wingdi.h>
#define MAX 512
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

void Add_Gaussian_Noise(BYTE *image) {
  // 가우시안 노이즈를 생성하여
}

void Edge_Detection_3(BYTE *image, int mask[][3], string output_name) {
  BITMAPHEADERS bh;
  generate_headers(bh);

  BYTE *noise = (BYTE *)malloc(sizeof(BYTE) * sizeof(image));
  memcpy(noise, image, sizeof(image));

  // 입력받은 마스크를 아용하여 Edge Detection을 진행하고, 결과영상을 만든다.
  // 추가로 계산된 오류값이 얼마인지도 콘솔로 출력해준다.
  // 콘솔로 출력하는 대신에 파일 이름으로 출력할 수 있다.

  // Original Image Edge Detection

  // Add Gaussian Noise

  // Noisy Image Edge Detection
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      
    }
  }

  // Calculate Edge Detection Error Rate
  reverse_raw_data(image);
  string PATH = "outputs/" + output_name + "EdgeDetection.bmp";
  FILE *output_file = fopen(PATH.c_str(), "wb");
  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);
  fwrite(image, sizeof(BYTE), MAX * MAX, output_file);
  fclose(output_file);
  // free(image);
}

void Stochastic(BYTE *image) {
  BITMAPHEADERS bh;
  generate_headers(bh);
  BYTE *noise = (BYTE *)malloc(sizeof(BYTE) * sizeof(image));
  memcpy(noise, image, sizeof(image));

  reverse_raw_data(image);
  string PATH = "outputs/Stochastic_EdgeDetection.bmp";
  FILE *output_file = fopen(PATH.c_str(), "wb");
  fwrite(&bh.hFile, sizeof(BITMAPFILEHEADER), 1, output_file);
  fwrite(&bh.hInfo, sizeof(BITMAPINFOHEADER), 1, output_file);
  fwrite(bh.hRGB, sizeof(RGBQUAD), 256, output_file);
  fwrite(image, sizeof(BYTE), MAX * MAX, output_file);
  fclose(output_file);
}

int main() {

  BYTE *image;
  FILE *input_file;
  input_file = fopen("lena_raw_512x512.raw", "rb");
  image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  fread(image, sizeof(BYTE), MAX * MAX, input_file);
  fclose(input_file);

  // Original Image Edge Detection

  // Add Gaussian Noise

  // Noisy Image Edge Detection

  // Calculate Edge Detection Error Rate

  return 0;
}