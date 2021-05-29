#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <fftw3.h>
#include <iostream>
#include <cmath>
#include <fstream>
using namespace std;

int main()
{
  int i;
  double y;
  int N = 50;
  double Fs = 1000;//sampling frequency
  double  T = 1 / Fs;//sample time 
  double f = 50;//frequency
  double* in;
  fftw_complex* out;
  double t[N];//time vector 
  double ff[N];
  fftw_plan plan_forward;

  in = (double*)fftw_malloc(sizeof(double) * N);
  out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

  for (int i = 0; i < N; i++)
  {
    t[i] = i * T;
    ff[i] = 1 / t[i];
    in[i] = 0.7 * sin(2 * M_PI * f * t[i]);// generate sine waveform
    double multiplier = 0.5 * (1 - cos(2 * M_PI * i / (N - 1)));//Hanning Window
    in[i] = multiplier * in[i];
  }

  plan_forward = fftw_plan_dft_r2c_1d(N, in, out, FFTW_ESTIMATE);

  fftw_execute(plan_forward);

  double v[N];

  for (int i = 0; i < N; i++)
  {

    v[i] = 20 * log(sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]) / N / 2);//Here I have calculated the y axis of the spectrum in dB

  }

  fstream myfile;

  myfile.open("example2.txt", fstream::out);

  myfile << "plot '-' using 1:2" << std::endl;

  for (i = 0; i < N; ++i)

  {

    myfile << ff[i] << " " << v[i] << std::endl;

  }

  myfile.close();

  fftw_destroy_plan(plan_forward);
  fftw_free(in);
  fftw_free(out);
  return 0;
}