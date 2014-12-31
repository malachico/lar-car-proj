#include <ros/ros.h>
//#include "component/ComponentMain.h"
//#include "component/ComponentStates.h"
#include <ros/spinner.h>
// #include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <sensor_msgs/image_encodings.h>
#include <boost/filesystem.hpp>
#include <sensor_msgs/CompressedImage.h>
#include <lar_msgs/Map.h>
#include <cv_bridge/cv_bridge.h>
#include <boost/thread/thread.hpp>
#include <std_msgs/Float64.h>
// #include <roadDetection/roadLanes.h>
// #include <roadDetection/lane.h>
#include <std_msgs/Char.h>
#include <opencv2/opencv.hpp>
#include <list>
#include <cmath>
#include "heightmap.h"
#include "helpermath.h"
#include "SimpleGUI.h"
#include <unistd.h>


using namespace cv;
using std::vector;
using lar_msgs::MapCell;
// using namespace roadDetection;
using namespace SimpleGUI;

//string CWD = "/src/lar-car-proj/scripts/";
string cwd;

ros::Publisher pubDrive;

/**
 * Frequency of doing stereo reconstruction and publishing height map
 */
const int MAPPING_FREQUENCY = 15; //Hz
// 
// ros::Publisher pubMap;

/**
 * Flags for existence of required components
 */
bool camR = false;
bool camL = false;
bool receivedLoc = false;
// bool disparityReady = false;
bool mapReady = false;
bool heightMapImgReady = false;
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
Mat camRImg, camLImg; 			//incoming images (input)
Vec3D pos, front, up, right; 		//derived from localization (input) 
Rotation rot;				//derived from localization (input)
HeightMap*  Map;    			//main height map
// Mat stereo;				//disparity map
// vector<lane> lanes;			//road lane data
Mat heightMapImg;



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
    heightMapImg = Map->buildGUI(0, -5, 0, 1);
    heightMapImgReady = true;
    gateway.unlock();
    //waitKey(1);
    //boost::this_thread::sleep(boost::posix_time::milliseconds(1000/MAPPING_FREQUENCY)); 
  }
}

/**
 * Input Message Handlers
 */
void handleMap(const lar_msgs::Map& msg)
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
  }
  
  vector<double>& heights = Map->getHeights();
  vector<int>& features = Map->getFeatures();
  for(int i = 0; i < 150; i++)
      for(int j = 0; j < 150; j++)
      {
	heights[j*150+i] = msg.data[j*150+i].height;
	//msg.data[j*150+i].type = types[j*150+i];
	features[j*150+i] = msg.data[j*150+i].feature;
      }
  
  mapReady = true;
  gateway.unlock();
}

bool isDriving = false;
void Connect(ImageButton* button, int x, int y)
{
  std_msgs::Float64 msg;
  if(!isDriving) 
  {
    isDriving = !isDriving;
    msg.data = 1;
    button->setString("Connected");
    ((SimpleButton*)button)->setColors(Scalar(60,60,60), Scalar(0,255,0), Scalar(0,0,0));
  }
  else 
  {
    isDriving = !isDriving;
    msg.data = 0;
    button->setString("Connect");
    ((SimpleButton*)button)->setColors(Scalar(60,60,60), Scalar(0,0,255), Scalar(0,0,0));
  }
  pubDrive.publish(msg);
  
}

void Script(ImageButton* button, int x, int y)
{
  string cwd = getenv("HOME");cwd += "/.ros/scripts/";
  cwd += button->getString();
  cwd += ".sh";
  system(cwd.c_str());
}

void UIThread()
{
	Canvas* canvas = new Canvas(1024, 768, false);
	canvas->setForegroundColor(Scalar(200,200,200));
	/*
	Mat m = imread("img.jpg", 1);
	Mat btn1 = imread("btn1.bmp", 1);
	Mat btn2 = imread("btn2.bmp", 1);
	Mat btn3 = imread("btn3.bmp", 1);
	*/
	
	
	CString* location = new CString("Location", 10, 30, 80);
	location->setColor(Scalar(0,0,0));
	canvas->addObject(location);
	
	CString* pos_x = new CString("x: 0", 10, 30, 110); 
	CString* pos_y = new CString("y: 0", 10, 30, 125); 
	CString* pose_yaw = new CString("Y: 0", 10, 30, 140); 
	pos_x->setColor(Scalar(0,0,0));
	canvas->addObject(pos_x);
	pos_y->setColor(Scalar(0,0,0));
	canvas->addObject(pos_y);
	pose_yaw->setColor(Scalar(0,0,0));
	canvas->addObject(pose_yaw);	
	
	CString* wayPoint = new CString("Way Point", 10, 30, 200);
	wayPoint->setColor(Scalar(0,0,0));
	canvas->addObject(wayPoint);
	
	CString* auto_manual = new CString("Auto/Manual", 10, 30, 300);
	auto_manual->setColor(Scalar(0,0,0));
	canvas->addObject(auto_manual);
	
	SimpleButton* manualAuto = new SimpleButton("Auto", 30, 315, 150, 200);
	manualAuto->setColors(Scalar(60,60,60), Scalar(120,120,120), Scalar(0,0,0));
	canvas->addObject(manualAuto);
	
	CString* connectstr = new CString("Connect", 10, 30, 520);
	connectstr->setColor(Scalar(0,0,0));
	canvas->addObject(connectstr);
	
	SimpleButton* connectButton = new SimpleButton("Connect", 30, 540, 150, 200);
	connectButton->setColors(Scalar(60,60,60), Scalar(0,0,255), Scalar(0,0,0));
	canvas->addObject(connectButton);
	connectButton->setListener(&Connect);
	
	CString* mapStr = new CString("HeightMap:", 10, 580, 330);
	mapStr->setColor(Scalar(255,0,0));
	canvas->addObject(mapStr);
	
	CString* camLStr = new CString("Left Cam:", 10, 500, 70);
	camLStr->setColor(Scalar(255,0,0));
	canvas->addObject(camLStr);
	
	CString* camRStr = new CString("Right Cam:", 10, 780, 70);
	camRStr->setColor(Scalar(255,0,0));
	canvas->addObject(camRStr);

	Image* heightMapImage = new Image(Mat(400, 500, CV_8UC3, Scalar(0,0,0)), 400, 350); //we gotta wait until we have the map
	canvas->addObject(heightMapImage);
	
	Image* camLImage = new Image(Mat(200, 250, CV_8UC3, Scalar(0,0,0)), 420, 90); //we gotta wait until we have the map
	canvas->addObject(camLImage);
	
	Image* camRImage = new Image(Mat(200, 250, CV_8UC3, Scalar(0,0,0)), 690, 90); //we gotta wait until we have the map
	canvas->addObject(camRImage);
	
	namespace fs = boost::filesystem;
	string cwd = getenv("HOME");cwd += "/.ros/scripts/";
	fs::path someDir(cwd);
	fs::directory_iterator end_iter;

	std::list<fs::path> files;

	if ( fs::exists(someDir) && fs::is_directory(someDir))
	{
	  for( fs::directory_iterator dir_iter(someDir) ; dir_iter != end_iter ; ++dir_iter)
	  {
	    if (fs::is_regular_file(dir_iter->status()) )
	    {
	      files.push_back(*dir_iter);
	    }
	  }
	}
	int pos_tmp = 0;
	for(std::list<fs::path>::iterator it = files.begin(); it != files.end(); it++)
	{
	  string buttonName = (*it).string();
	  if(buttonName.substr((buttonName.length()-3), buttonName.length()) != ".sh") continue;
	  buttonName = buttonName.substr(0, buttonName.rfind(".")).substr(buttonName.rfind("/")+1, buttonName.length());
	  SimpleButton* btn = new SimpleButton(buttonName, pos_tmp, 10, (4+buttonName.length())*12, 30);
	  canvas->addObject(btn);
	  btn->setListener(&Script);
	  pos_tmp += (4+buttonName.length())*12;
	}
	SimpleGUI::GUIWindow wnd(canvas, "LAR");
	wnd.update();
	while(1)
	{	
		gateway.lock();
		bool haveMapImage = heightMapImgReady;
		gateway.unlock();
		if(haveMapImage)
		{
		    heightMapImage->setImage(heightMapImg);
		}
		inputData.lock();
		if(camL)
		  camLImage->setImage(camLImg);
		if(camR)
		  camRImage->setImage(camRImg);
		if(receivedLoc)
		{
		  char str[50];
		  sprintf(str, "x: %.3f", pos.x);
		  pos_x->setString(str);
		  sprintf(str, "y: %.3f", pos.y);
		  pos_y->setString(str);
		  sprintf(str, "y: %.3f", rot.yaw*180/3.14159);
		  pose_yaw->setString(str);
		}
		inputData.unlock();
		wnd.update();
		wnd.draw();	
		boost::this_thread::sleep(boost::posix_time::milliseconds(10));
	}
}

int main(int argc,char** argv)
{
  cwd = argv[0];
  
  
  Map = new HeightMap(400, 400);
  Map->setBlendFunc(HeightMap::maxHeightFilter);
  
  ros::init(argc, argv, "decision_making");
//   namedWindow("stereo", 1);
  //setMouseCallback("stereo", MouseCallBack, NULL);
  ros::NodeHandle n;
  ros::Subscriber map = n.subscribe("/PER/Map", 10, handleMap);
  ros::Subscriber camL = n.subscribe("SENSORS/CAM/L/compressed", 10, handleCamCompressed_L);
  ros::Subscriber camR = n.subscribe("SENSORS/CAM/R/compressed", 10, handleCamCompressed_R);
  
  pubDrive = n.advertise<std_msgs::Float64>("/Drive", 10);
  boost::thread t3(VisualThread);
  
  boost::thread UI(UIThread);
  
  ros::AsyncSpinner spinner(1);
  spinner.start();
  
  
  

  ros::waitForShutdown();
//   delete Map;
  return 0;
}
