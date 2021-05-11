#include <iostream>
using namespace std;

/* The data type for input _image should be Int */
variance = get_image_power(input_image);
stddev_noise=sqrt(variance/pow(10.0,((double) SNR/10));  /* Here SNR is set at 8 */
AddGaussianNoise(input_image, noise_image, stddev_noise);

AddGaussianNoise(input_img, noise_img, sigma)  /*Add Gaussian Noise to the input image */
int input_img[][N], noise_img[][N];
double sigma;
{
  int i, j, s;
  for (i = 0; i < N; i++)
    for (j = 0; j < N; j++) {
      s = input_img[i][j] + Gaussian(sigma);
      noise_img[i][j] = s > 255 ? 255 : s < 0 ? 0 : s;
    }
}
float Gaussian(sd)
float sd;
{
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
  return (gaus * sd);
}
