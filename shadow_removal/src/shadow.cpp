#include "ros/ros.h"
#include "std_msgs/String.h"
#include <sensor_msgs/CompressedImage.h>
#include <opencv2/opencv.hpp>
#include <stdio.h>
#include "s_removal/s_removal.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <dirent.h>


using namespace cv;
using namespace std;

/** *
 * 		displayImage:
 ** */
ros::NodeHandle *n;
ros::Publisher chatter_pub ;

void displayImage(const sensor_msgs::CompressedImage& msg)
// void displayImage(const sensor_msgs::Image& msg)
{
  Mat m = imdecode(Mat(msg.data),1);

//   Mat m = imread("Desktop/lar_ws/src/lar-car-proj/shadow_removal/src/pics/shadow.jpg", 1);
  
  vector<double> lanes = remove_shadow(m);
  
  //chatter_pub.publish(retMsg);
}

/** *
 * 		chatterCallback:
 ** */
  
int counter = 0;
int everyNthTime = 10;

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
  ros::init(argc, argv, "listener");

  ros::NodeHandle n1;
  n = &n1;
  ros::Subscriber sub = n->subscribe("SENSORS/CAM/R/compressed", 1000, chatterCallback);
  //chatter_pub = n->advertise<shadow_removal::roadLanes>("RoadLanes",1000);
  ros::AsyncSpinner spinner(1);
  spinner.start();
  ros::waitForShutdown();
  
  return 0;
}
