#include <ros/ros.h>
//#include "component/ComponentMain.h"
//#include "component/ComponentStates.h"
#include <ros/spinner.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/CompressedImage.h>
#include <stereo_package/Map.h>
#include <cv_bridge/cv_bridge.h>
#include <boost/thread/thread.hpp>
#include <road_detection/roadLanes.h>
#include <road_detection/lane.h>
#include <std_msgs/Char.h>
#include <opencv2/opencv.hpp>
#include <cmath>
#include "stereo.h"
#include "heightmap.h"
#include "helpermath.h"

using namespace cv;
using std::vector;
using namespace road_detection;



/**
 * Frequency of doing stereo reconstruction and publishing height map
 */
const int MAPPING_FREQUENCY = 15; //Hz

ros::Publisher pubMap;

/**
 * Flags for existence of required components
 */
bool camR = false;
bool camL = false;
bool receivedLoc = false;
bool disparityReady = false;
bool mapReady = false;
bool lanesReady = false;

/**
 * Synchronization mechanisms
 */
boost::mutex inputData; //sync between incoming messages and the threads that read them (lock images, vectors, rotation)
boost::mutex disparity; //sync between stereo thread and mapper thread (lock disparity map)
boost::mutex gateway;   //sync between mapper thread (writer) and visual & publish threads (readers) (lock height map)

/**
 * Global variables to be accessed by the various worker threads
 */
Mat camRImg, camLImg; 			//incoming images (input)
Vec3D pos, front, up, right; 		//derived from localization (input) 
Rotation rot;				//derived from localization (input)
HeightMap*  Map;    			//main height map
Mat stereo;				//disparity map
vector<lane> lanes;			//road lane data


/**
 * Input Message Handlers
 */
  void handleLocalization(const geometry_msgs::PoseWithCovarianceStamped& msg)
  {
    inputData.lock();
    pos = Vec3D(msg.pose.pose.position.x,msg.pose.pose.position.y,msg.pose.pose.position.z);
    Quaternion quat(msg.pose.pose.orientation.x,msg.pose.pose.orientation.y,msg.pose.pose.orientation.z,msg.pose.pose.orientation.w);
    up = GetUpVector(quat.x,quat.y,quat.z,quat.w);
    right = GetRightVector(quat.x,quat.y,quat.z,quat.w);
    front = GetFrontVector(quat.x,quat.y,quat.z,quat.w);
    rot = GetRotation(quat);
    receivedLoc = true;
    inputData.unlock();
  }

  void handleCamCompressed_L(const sensor_msgs::CompressedImage& msg)
  {
    Mat img = imdecode(Mat(msg.data),1);
    inputData.lock();
    camLImg = img;
    camL = true;
    inputData.unlock();
  }

  void handleCamCompressed_R(const sensor_msgs::CompressedImage& msg)
  {
    Mat img = imdecode(Mat(msg.data),1);
    inputData.lock();
    camRImg = img;
    camR = true;
    inputData.unlock();
  } 
  
  void handleWalrusData(const road_detection::roadLanes& msg)
  {
    inputData.lock();
    lanes = msg.lanes;
    lanesReady = true;
    inputData.unlock();
  }
   
/**
 */    
    
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

/**
 * Applies stereo matching algorithms to get a disparity map
 */
void StereoThread()
{
  while(1)
  {
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000/MAPPING_FREQUENCY)); 
    
    Mat _stereo;
    inputData.lock();
    if(!camR || !camL || !receivedLoc) //all data arrived at least once
    {
      inputData.unlock();
      continue;
    }
    _stereo = handleStereo(camLImg, camRImg);
    inputData.unlock();
    
    disparity.lock();
    stereo = _stereo;
    disparityReady = true;
    disparity.unlock(); 
  }
}

/**
 * Prepares the necessary data for publishing to ROS
 * and does the publishing.
 */
void ROSThread()
{
  while(1)
  {
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000/MAPPING_FREQUENCY)); 
    
    inputData.lock();
      if(!receivedLoc)
      {
	inputData.unlock();
	continue;
      }
      Quaternion myQuat = GetFromRPY(rot);
      Vec3D position = pos;
    inputData.unlock();
    
    
    stereo_package::Map msg;
    static int seq = 0;
    msg.header.seq = seq++;
    msg.header.stamp.sec = ros::Time::now().sec;
    msg.header.stamp.nsec = ros::Time::now().nsec;
    msg.header.frame_id = "map";
    
    msg.info.map_load_time.sec = ros::Time::now().sec;
    msg.info.map_load_time.nsec = ros::Time::now().nsec;
    msg.info.width = 150;
    msg.info.height = 150;
    msg.info.resolution = 0.2;
    msg.info.origin.position.x = position.x;
    msg.info.origin.position.y = position.y;
    msg.info.origin.position.z = position.z;
    msg.info.origin.orientation.x = myQuat.x;
    msg.info.origin.orientation.y = myQuat.y;
    msg.info.origin.orientation.z = myQuat.z;
    msg.info.origin.orientation.w = myQuat.w;
    msg.data.resize(150*150);
    
    gateway.lock();
      if(!mapReady)
      {
	gateway.unlock();
	continue;
      }
      HeightMap Oded = Map->deriveMap(position.x, position.y, rot);
      vector<double>& heights = Oded.getHeights();
      //vector<int>& types = Oded.getTypes();
      vector<int>& features = Oded.getFeatures();
      for(int i = 0; i < 150; i++)
	for(int j = 0; j < 150; j++)
	{
	  msg.data[j*150+i].height = heights[j*150+i];
	  //msg.data[j*150+i].type = types[j*150+i];
	  msg.data[j*150+i].feature = features[j*150+i];
	}
    gateway.unlock();
    pubMap.publish(msg);
  }
}

/**
 * Projects the disparity map onto a height map.
 */
void MappingThread()
{
  while(1)
  {
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000/MAPPING_FREQUENCY)); 
    
    inputData.lock();
    Vec3D _right = right;
    Vec3D _front = front;
    Vec3D _up = up;
    Vec3D _pos = pos;
    vector<lane> _lanes = lanes;
    inputData.unlock();
    
    disparity.lock();
    if(!disparityReady) 
    {
      disparity.unlock();
      continue;
    }
    Mat _stereo = stereo;
    disparity.unlock();
    
    gateway.lock();
    ProjectDepthImage(Map, _stereo, _right, _front, _up, _pos, _lanes);
    lanes.clear();
    mapReady = true;
    gateway.unlock();
  }
}

/**
 * Draws the disparity map and the height map.
 */
void VisualThread()
{
  while(1)
  {
    boost::this_thread::sleep(boost::posix_time::milliseconds(1000/MAPPING_FREQUENCY));
    
    inputData.lock();
    if(!receivedLoc)
    {
      inputData.unlock();
      continue;
    }
    Vec3D _pos = pos;
    Rotation _rot = rot;
    inputData.unlock();
    
    gateway.lock();
    if(!mapReady)
    {
      gateway.unlock();
      continue;
    }
    Map->displayGUI(_rot.yaw*180/3.14159,_pos.x,_pos.y, 2);
    gateway.unlock();
    waitKey(1);
    //boost::this_thread::sleep(boost::posix_time::milliseconds(1000/MAPPING_FREQUENCY)); 
  }
}

int main(int argc,char** argv)
{
  Map = new HeightMap(400, 400);
  Map->setBlendFunc(HeightMap::maxHeightFilter);
  
  ros::init(argc, argv, "stereo_package");
  namedWindow("stereo", 1);
  //setMouseCallback("stereo", MouseCallBack, NULL);
  ros::NodeHandle n;
  ros::Subscriber camL = n.subscribe("SENSORS/CAM/L/compressed", 10, handleCamCompressed_L);
  ros::Subscriber camR = n.subscribe("SENSORS/CAM/R/compressed", 10, handleCamCompressed_R);
  ros::Subscriber loc = n.subscribe("LOC/Pose", 10, handleLocalization);
  ros::Subscriber wlrs = n.subscribe("RoadLanes", 10, handleWalrusData);
  pubMap = n.advertise<stereo_package::Map>("PER/Map", 10);
  
  boost::thread t1(StereoThread);
  boost::thread t2(MappingThread);
  boost::thread t3(VisualThread);
  boost::thread t4(ROSThread);
  
  
  ros::AsyncSpinner spinner(4); // Use 4 threads
  spinner.start();
  
  

  ros::waitForShutdown();
  delete Map;
  return 0;
}
