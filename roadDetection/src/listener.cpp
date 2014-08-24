#include "ros/ros.h"
#include "std_msgs/String.h"
#include "sensor_msgs/Image.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "displayImage/DisplayImage.h"
#include "displayImage/polyfit.h"
#define _USE_MATH_DEFINES
#include <cmath>

using namespace cv;
using namespace std;

/** *
 * 		get_distance:
 ** */
#define FOV 		1.3963
#define CAM_HEIGHT 	1.97
#define WIDTH 		4872
#define LENGTH		1224
void get_distance(int picX, int picY)
{
  float xDist, yDist;
  float alpha = (M_PI-FOV)/2.0;
  float closest = CAM_HEIGHT*(tan(alpha));
  float beta = (picY / (LENGTH/2.0))*( FOV/2 );
  if(picX==0)
    xDist = 1000;
  else
    xDist = (WIDTH/2)*(closest/picX);
  yDist = xDist*(tan(beta));
  
  printf("(xDist: %f, yDist: %f)\n", xDist, yDist);
}

/** *
 * 		displayImage:
 ** */
void displayImage(const sensor_msgs::Image msg)
{
  //*
  int rows = msg.height;
  int colsMul3 = msg.step;
  
  Mat m = Mat(rows, colsMul3/3, CV_8UC3);
  
  for(int i=0; i< rows; i++)
  {
    for(int j=0; j< colsMul3/3; j++)
    {
      m.at<Vec3b>(i,j)[2] = msg.data[i*colsMul3 + j*3 + 0];
      m.at<Vec3b>(i,j)[1] = msg.data[i*colsMul3 + j*3 + 1];
      m.at<Vec3b>(i,j)[0] = msg.data[i*colsMul3 + j*3 + 2];
    }
  }
  //*/
  /*
  
  Mat m = imread("image.jpg");
  if(m.empty())
  {
    printf("returning\n");
    return;
  }
  else
  {
    printf("here\n");
  }
  //*/
  detectRoad(m, 50, 100);
  
}

/** *
 * 		chatterCallback:
 ** */
int counter = 0;
int everyNthTime = 10;

void chatterCallback(const sensor_msgs::Image msg)
{
  if(counter%everyNthTime == 0)
//   if(counter == 0)
  {
    displayImage(msg);
    counter = 1;
  }
  else
  {
    counter ++;
  }
    
}


/** *
 * 		main:
 ** */
int main(int argc, char **argv)
{
  ros::init(argc, argv, "listener");

  ros::NodeHandle n;

  ros::Subscriber sub = n.subscribe("SENSORS/CAM/L", 1000, chatterCallback);
//   printf("spin\n");
  ros::spin();

  return 0;
}
