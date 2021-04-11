#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <windows.h>
#include <wingdi.h>
#define MAX 512
using namespace std;

void generate_raw_data(BYTE *image, bool flag) {
  /*
  RAW 파일의 데이터를 생성하는 부분이다.
  flag가 true이면 그래프와 유사하게 그린 이미지가 생성된다.
  flag가 false이면 급격하게 설정된 값으로 바뀐다.
  Mach band를 극적으로 느끼기 위해 기본적으로 false 상태이다.
  */
  BYTE val;
  for (int i = 0; i < MAX; i++) {
    for (int j = 0; j < MAX; j++) {
      if (0 <= j && j < 100) {
        val = 120;
      } else if (100 <= j && j < 200) {
        val = flag ? 120 + (15.0 / 100) * (j - 100) : 135;
      } else if (200 <= j && j < 280) {
        val = flag ? 135 + (90.0 / 80) * (j - 200) : 225;
      } else if (280 <= j && j < 300) {
        val = flag ? 225 + (15.0 / 20) * (j - 280) : 240;
      } else if (300 <= j && j < MAX) {
        val = 240;
      }
      image[i * MAX + j] = val;
    }
  }
}

int main() {
  bool image_style_flag = false;
  BYTE *image = (BYTE *)malloc(sizeof(BYTE) * MAX * MAX);
  FILE *fp;
  fp = fopen("outputs/Mach_band_512x512.raw", "wb");
  if (fp == NULL) {
    printf("File Open Error\n");
    return 0;
  }

  generate_raw_data(image, image_style_flag);
  fwrite(image, sizeof(BYTE), MAX * MAX, fp);
  fclose(fp);
  return 0;
}
