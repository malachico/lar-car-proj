#include "ros/ros.h"
#include "std_msgs/String.h"
#include "std_msgs/Header.h"
#include <sensor_msgs/CompressedImage.h>
// #include <sensor_msgs/Image.h>
#include "std_msgs/Float64MultiArray.h"
// #include "per/roadLanes.h"
// #include "per/lane.h"
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "displayImage/entropy.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <time.h>       /* clock_t, clock, CLOCKS_PER_SEC */

using namespace cv;
using namespace std;
// using namespace per;

/** *
 * 		displayImage:
 ** */
ros::NodeHandle *n;
ros::Publisher chatter_pub ;
bool lock = false;
int toDebug;
// int image = 4;
void displayImage(const sensor_msgs::CompressedImage& msg)
{
  if(lock)
  {
    cout << "locked" << endl;
    return;
  }
  Mat m = imdecode(Mat(msg.data),1);
//   char s[100] = {0};
//   sprintf(s,"ent/image%d.jpg",image);
//   printf("%s\n",s);
//   Mat m = 
//   cv::imread(s);//imdecode(Mat(msg.data),1);
//   image++;
//   cv::resize(m, m, img.size());
  Mat lanes;
  lock = true;
  
  clock_t t;
  t = clock();
  lanes = displayMyEntropy/*detectRoad*/(m, 50, 100, toDebug);
  t = clock() - t;
  printf ("%.3f seconds.\n", ((float)t)/CLOCKS_PER_SEC);
  
  lock = false;
  sensor_msgs::CompressedImage retMsg;
  retMsg.header = std_msgs::Header();
  retMsg.format = "png";
  
  std::vector<int> params;
  params.resize(3, 0);
  params[0] = CV_IMWRITE_PNG_COMPRESSION;
  params[1] = 2; 
  try
  {
     cv::imencode(".png", lanes, retMsg.data, params);
  }
  catch (cv::Exception& e)
  {
    ROS_ERROR("%s", e.what());
  }
  chatter_pub.publish(retMsg);
}

/** *
 * 		chatterCallback:
 ** */
  
int counter = 0;
int everyNthTime = 5;//should be : 2;

void chatterCallback(const sensor_msgs::CompressedImage& msg)
{
  
  if(counter%everyNthTime == 0)
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
  if(argc > 1)
    if(strcmp(argv[1], "-d"))
      toDebug = 0;
    else
      toDebug =1; 
  ros::init(argc, argv, "listener");
  ros::NodeHandle n1;
  n = &n1;
  ros::Subscriber sub = n->subscribe("SENSORS/CAM/L/compressed", 1000, chatterCallback);
  chatter_pub = n->advertise<sensor_msgs::CompressedImage&>("RoadLanes",1000);
  ros::AsyncSpinner spinner(1);
  spinner.start();
  ros::waitForShutdown();
  return 0;
}
