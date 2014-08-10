#include <ros/ros.h>
//#include "component/ComponentMain.h"
//#include "component/ComponentStates.h"
#include <ros/spinner.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/CompressedImage.h>
#include <cv_bridge/cv_bridge.h>
#include <boost/thread/thread.hpp>
#include <std_msgs/Char.h>
#include <opencv2/opencv.hpp>
#include <cmath>
#include "component/rdbg.h"
#include "stereo.h"
#include "heightmap.h"

using namespace cv;

/**
 * Flags for receipt of initial required components
 */
bool camR = false;
bool camL = false;
bool receivedLoc = false;

Mat camRImg, camLImg;
HeightMap*  Map;
Mat Q;
Mat stereo;
Vec3D pos, front, up, right;
Rotation rot;


void handleLocalization(const geometry_msgs::PoseWithCovarianceStamped& msg)
{
  pos = Vec3D(msg.pose.pose.position.x,msg.pose.pose.position.y,msg.pose.pose.position.z);
  Quaternion quat(msg.pose.pose.orientation.x,msg.pose.pose.orientation.y,msg.pose.pose.orientation.z,msg.pose.pose.orientation.w);
  up = GetUpVector(quat.x,quat.y,quat.z,quat.w);
  right = GetRightVector(quat.x,quat.y,quat.z,quat.w);
  front = GetFrontVector(quat.x,quat.y,quat.z,quat.w);
  rot = GetRotation(quat);
  receivedLoc = true;
}

void handleCamCompressed_L(const sensor_msgs::CompressedImage& msg)
{
  camLImg = imdecode(Mat(msg.data),1);
  camL = true;
}

void handleCamCompressed_R(const sensor_msgs::CompressedImage& msg)
{
  camRImg = imdecode(Mat(msg.data),1);
  camR = true;
  
  if(camR && camL && receivedLoc)
  {
    stereo = handleStereo(camLImg, camRImg);
    ProjectDepthImage(Map, stereo, right, front, up, pos);
    //ProjectDepthImage3(Map, stereo, camRImg, Q, Vec3D(1,0,0), Vec3D(0,1,0), Vec3D (0,0,1), Vec3D(0,0,0));
    Map->displayGUI(rot.yaw*180/3.14159,pos.x,pos.y, 1);
    waitKey(1);
   // Mat out;
   // reprojectImageTo3D(stereo, out, InputArray Q);
  }
} 

void handleCamR(const sensor_msgs::Image& msg)
{
  cv_bridge::CvImagePtr cv_ptr;
  try
  {
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }
  camRImg = cv_ptr->image;
  camR = true;
  if(camR && camL)
  {
    Mat stereo = handleStereo(camLImg, camRImg);
    //ProjectDepthImage(Map, stereo, Vec3D(1,0,0), Vec3D(0,1,0), Vec3D (0,0,1), Vec3D(0,0,0));
    //Map->displayGUI(0,0,0, 2);
    //waitKey(1);
  }
}

void handleCamL(const sensor_msgs::Image& msg)
{
  cv_bridge::CvImagePtr cv_ptr;
  try
  {
    cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::BGR8);
  }
  catch (cv_bridge::Exception& e)
  {
    ROS_ERROR("cv_bridge exception: %s", e.what());
    return;
  }
  camLImg = cv_ptr->image;
  camL = true;
}
  
void MouseCallBack(int event, int x, int y, int flags, void* userdata)
{
  if  ( event == EVENT_LBUTTONDOWN )
  {
    int pxl = stereo.at<uchar>(y,x);
    printf("value: %d\n", pxl);
  }
  else if  ( event == EVENT_RBUTTONDOWN )
  {
    //cout << "Right button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
  }
  else if  ( event == EVENT_MBUTTONDOWN )
  {
    //cout << "Middle button of the mouse is clicked - position (" << x << ", " << y << ")" << endl;
  }
  else if ( event == EVENT_MOUSEMOVE )
  {
    //cout << "Mouse move over the window - position (" << x << ", " << y << ")" << endl;
  }
}
  
int main(int argc,char** argv)
{
  Map = new HeightMap(400, 400);
  Map->setBlendFunc(HeightMap::maxHeightFilter);
  cv::FileStorage fs; // reading a Q matrix that tell us the stereo calibration
  bool opened = fs.open("Q.xml", FileStorage::READ);
  system("pwd");
  if(!opened) printf("Failed to open Q.xml\n");
  fs["Q"] >> Q;
  ros::init(argc, argv, "stereo_package");
  namedWindow("stereo", 1);
  setMouseCallback("stereo", MouseCallBack, NULL);
  ros::NodeHandle n;
  ros::Subscriber camL = n.subscribe("SENSORS/CAM/L/compressed", 10, handleCamCompressed_L);
  ros::Subscriber camR = n.subscribe("SENSORS/CAM/R/compressed", 10, handleCamCompressed_R);
  ros::Subscriber loc = n.subscribe("LOC/Pose", 10, handleLocalization);
  
  ros::AsyncSpinner spinner(4); // Use 4 threads
  spinner.start();
  
  

  ros::waitForShutdown();
  delete Map;
  return 0;
}
