
#include <ros/ros.h>
#include "sensor_msgs/NavSatFix.h"
#include "sensor_msgs/Imu.h"
#include "std_msgs/Float64.h"
#include "ekf_class.h"
#include <opencv2/opencv.hpp>
#include "userHeader.h"
#include "noiseless_estimator.h"
#include "lar_msgs/GpsSpeed.h"
using namespace std;
using namespace cv;


const double FREQUENCY = 10;
ekf _estimator;
Observer _observer;

ros::Publisher loc_publisher,spd_publisher;
void GPSCallback(const sensor_msgs::NavSatFix& msg)
{
	if(_added_noise)
		_estimator.setGPSMeasurement(msg);
	else
		_observer.setGPSMeasurement(msg);
}

void IMUCallback(const sensor_msgs::Imu& msg)
{
	if(_added_noise)
		_estimator.setIMUMeasurement(msg);
	else
		_observer.setIMUMeasurement(msg);
}
void GasPedalStateCallback(const std_msgs::Float64& msg)
{
	_estimator.setGasPedalState(msg);
}
void GpsSpeedCallback(const lar_msgs::GpsSpeed& msg)
{
	if(_added_noise)
		_estimator.setGPSSpeedMeasurement(msg);
	else
		_observer.setGPSSpeedMeasurement(msg);
}
void publish_location_speed()
{
//   cout << "publsih" << endl;
  if(_added_noise)
  {
    _estimator.estimator();
    loc_publisher.publish(_estimator.getEstimatedPose());
    spd_publisher.publish(_estimator.getEstimatedSpeed());
  }
  else
  {
    _observer.estimator();
    loc_publisher.publish(_observer.getEstimatedPose());
    spd_publisher.publish(_observer.getEstimatedSpeed());
  }
//   cout << "out " << endl;
}
int main(int argc, char **argv)
{
  cout << "EKF initializing..." << endl;
  ros::init(argc, argv, "ekf");
  ros::NodeHandle n;
  /**
   * subscribe to GPS, IMU and the speed topic.
   */
  ros::Subscriber gps_sub = n.subscribe("/SENSORS/GPS", 1000, GPSCallback);
  ros::Subscriber imu_sub = n.subscribe("/SENSORS/IMU", 1000, IMUCallback);
  ros::Subscriber gps_speed_sub = n.subscribe("/SENSORS/GPS/Speed", 1000, GpsSpeedCallback);
  //ros::Subscriber pedal_sub = n.subscribe("/drivingVic/gas_pedal/state", 1000, GasPedalStateCallback);
  /**
   * Generate publishers - location and speed
   */
  loc_publisher = n.advertise<geometry_msgs::PoseWithCovarianceStamped>("/LOC/Pose",FREQUENCY);
  spd_publisher = n.advertise<geometry_msgs::TwistStamped>("/LOC/Velocity",FREQUENCY);
  ros::Rate loop_rate(FREQUENCY); // loop rate of 10Hz

  while(ros::ok())
  {
    publish_location_speed();
    ros::spinOnce();
    loop_rate.sleep();
  }
  
  return 0;
}
