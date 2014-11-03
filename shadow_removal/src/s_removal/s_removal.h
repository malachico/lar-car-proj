#ifndef DISPLAY_IMAGE_H
#define DISPLAY_IMAGE_H

#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>
#include <cmath>
#include <vector>

using namespace std;
using namespace cv;

/** *******************************
 *  detectRoad:
 ** *******************************/
vector<double> remove_shadow(Mat read_image);


#endif 