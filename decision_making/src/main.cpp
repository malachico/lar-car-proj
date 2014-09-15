#include <ros/ros.h>
//#include "component/ComponentMain.h"
//#include "component/ComponentStates.h"
#include <ros/spinner.h>
// #include <geometry_msgs/PoseWithCovarianceStamped.h>
// #include <sensor_msgs/image_encodings.h>
// #include <sensor_msgs/CompressedImage.h>
#include <stereo_package/Map.h>
#include <cv_bridge/cv_bridge.h>
#include <boost/thread/thread.hpp>
// #include <roadDetection/roadLanes.h>
// #include <roadDetection/lane.h>
#include <std_msgs/Char.h>
#include <opencv2/opencv.hpp>
#include <cmath>
#include "heightmap.h"
#include "helpermath.h"

using namespace cv;
using std::vector;
using stereo_package::MapCell;
// using namespace roadDetection;



/**
 * Frequency of doing stereo reconstruction and publishing height map
 */
const int MAPPING_FREQUENCY = 15; //Hz
// 
// ros::Publisher pubMap;

/**
 * Flags for existence of required components
 */
// bool camR = false;
// bool camL = false;
bool receivedLoc = false;
// bool disparityReady = false;
bool mapReady = false;
// bool lanesReady = false;

/**
 * Synchronization mechanisms
 */
boost::mutex inputData; //sync between incoming messages and the threads that read them (lock images, vectors, rotation)
// boost::mutex disparity; //sync between stereo thread and mapper thread (lock disparity map)
boost::mutex gateway;   //sync between mapper thread (writer) and visual & publish threads (readers) (lock height map)
// 
// /**
//  * Global variables to be accessed by the various worker threads
//  */
// Mat camRImg, camLImg; 			//incoming images (input)
Vec3D pos, front, up, right; 		//derived from localization (input) 
Rotation rot;				//derived from localization (input)
HeightMap*  Map;    			//main height map
// Mat stereo;				//disparity map
// vector<lane> lanes;			//road lane data


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
    Map->displayGUI(_rot.yaw*180/3.14159,_pos.x,_pos.y, 1);
    gateway.unlock();
    waitKey(1);
    //boost::this_thread::sleep(boost::posix_time::milliseconds(1000/MAPPING_FREQUENCY)); 
  }
}

/**
 * Input Message Handlers
 */
void handleMap(const stereo_package::Map& msg)
{
  geometry_msgs::Pose pose = msg.info.origin;
  inputData.lock();
  pos = Vec3D(pose.position.x, pose.position.y, pose.position.z);
  Quaternion myQuat = Quaternion(pose.orientation.x,pose.orientation.y,pose.orientation.z,pose.orientation.w);
  rot = GetRotation(myQuat);
  receivedLoc = true;
  inputData.unlock();
  
  vector<MapCell> data = msg.data;
  gateway.lock();
  bool isReady = mapReady;
  if(!isReady)
  {
    //acquire the info and create the map only once
    nav_msgs::MapMetaData info = msg.info;
    Map = new HeightMap(info.width, info.height);
    vector<double>& heights = Map->getHeights();
    for(int i = 0; i < 150; i++)
	for(int j = 0; j < 150; j++)
	{
	  heights[j*150+i] = msg.data[j*150+i].height;
	  //msg.data[j*150+i].type = types[j*150+i];
	  //msg.data[j*150+i].feature = features[j*150+i];
	}
  }
  else
  {
    vector<double>& heights = Map->getHeights();
    for(int i = 0; i < 150; i++)
	for(int j = 0; j < 150; j++)
	{
	  heights[j*150+i] = msg.data[j*150+i].height;
	  //msg.data[j*150+i].type = types[j*150+i];
	  //msg.data[j*150+i].feature = features[j*150+i];
	}
  }
  mapReady = true;
  gateway.unlock();
}

int main(int argc,char** argv)
{
  Map = new HeightMap(400, 400);
  Map->setBlendFunc(HeightMap::maxHeightFilter);
  
  ros::init(argc, argv, "decision_making");
//   namedWindow("stereo", 1);
  //setMouseCallback("stereo", MouseCallBack, NULL);
  ros::NodeHandle n;
  ros::Subscriber map = n.subscribe("/PER/Map", 10, handleMap);
  
  boost::thread t3(VisualThread);
  
  ros::AsyncSpinner spinner(1);
  spinner.start();
  
  

  ros::waitForShutdown();
//   delete Map;
  return 0;
}
