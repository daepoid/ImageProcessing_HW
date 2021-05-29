#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/types_c.h"
#include <iostream>
using namespace std;
using namespace cv;

//project main function
int main(int argc, char** argv) {

  //create image window
  namedWindow("image", 1);

  //create test Mat, 400 x 400
  Mat testMat = cv::Mat::zeros(200, 850, CV_8UC3);

  //write text
  putText(testMat, "HELLO WORLD!! HEISANBUG", cvPoint(100, 100),
    FONT_HERSHEY_PLAIN, 3, cvScalar(0, 255, 255), 4);

  //show image
  imshow("image", testMat);
  waitKey(0);

  //close all windows
  destroyAllWindows();

  return 0;
}
