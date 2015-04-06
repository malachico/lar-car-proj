#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <opencv2/highgui/highgui.hpp>
#include <cv_bridge/cv_bridge.h>
#include <string>
#include <sensor_msgs/CompressedImage.h>
ros::NodeHandle *n;
cv::VideoWriter outputVideo,outputVideo1;
bool cap1=false,cap0=false;
int cntr = 0;
void compressedImageCallback(const sensor_msgs::CompressedImage& msg)
{
 // std::cout << msg.format << std::endl;
    cv::Mat im = imdecode(cv::Mat(msg.data),1);
    
//     outputVideo << im;
//     cv::imshow("view", im);
//     cv::waitKey(30);
    char name[20]={0};
    sprintf(name,"%04dl.jpeg",cntr);
    cv::imwrite(name,im);
    if(cap0)
    {
      cntr++;
      cap1 = false;
      cap0 = false;
    }
    else
      cap1 = true;
}
void compressedImageCallback1(const sensor_msgs::CompressedImage& msg)
{
 // std::cout << msg.format << std::endl;
    cv::Mat im = imdecode(cv::Mat(msg.data),1);
        char name[20]={0};
    sprintf(name,"%04dr.jpeg",cntr);
    cv::imwrite(name,im);
    if(cap1)
    {
      cntr++;
      cap1 = false;
      cap0 = false;
    }
    else
      cap0 = true;
//     outputVideo1 << im;
//     cv::imshow("view2", im);
//     cv::waitKey(30);
    
}

void ImageCallback(const sensor_msgs::ImageConstPtr& msg)
{
  try
  {
    cv::Mat im = cv_bridge::toCvShare(msg, "bgr8")->image;
    char name[20]={0};
    sprintf(name,"%04dr.png",cntr);
    cv::imwrite(name,im);
    if(cap1)
    {
      cntr++;
      cap1 = false;
      cap0 = false;
    }
    else
      cap0 = true;
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
}

void ImageCallback1(const sensor_msgs::ImageConstPtr& msg)
{
  try
  {
    cv::Mat im = cv_bridge::toCvShare(msg, "bgr8")->image;
    char name[20]={0};
    sprintf(name,"%04dl.png",cntr);
    cv::imwrite(name,im);
    if(cap0)
    {
      cntr++;
      cap1 = false;
      cap0 = false;
    }
    else
      cap1 = true;
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("Could not convert from '%s' to 'bgr8'.", msg->encoding.c_str());
  }
}

int main(int argc, char **argv)
{
  outputVideo.open("Right.mpg",CV_FOURCC('P','I','M','1'),30,cv::Size(1288,964));
  outputVideo1.open("Left.mpg",CV_FOURCC('P','I','M','1'),30,cv::Size(1288,964));
  ros::init(argc, argv, "image_listener");
  ros::NodeHandle nh;
  //cv::namedWindow("view");cv::namedWindow("view2");
  //cv::startWindowThread();
 ros::Subscriber sub3 = nh.subscribe("/SENSORS/FLEA3/0/compressed", 1, compressedImageCallback);
ros::Subscriber sub4 = nh.subscribe("/SENSORS/FLEA3/1/compressed", 1, compressedImageCallback1);

//  ros::Subscriber sub3 = nh.subscribe("/SENSORS/FLEA3/0", 1, ImageCallback);
// ros::Subscriber sub4 = nh.subscribe("/SENSORS/FLEA3/1", 1, ImageCallback1);
  ros::spin();
  cv::destroyWindow("view");
}