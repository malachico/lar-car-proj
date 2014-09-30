#include <math.h>
#include "ros/ros.h"
#include <opencv2/opencv.hpp>
#include "sensor_msgs/NavSatFix.h"
#include "sensor_msgs/Imu.h"
#include "geometry_msgs/PoseWithCovarianceStamped.h"
#include "geometry_msgs/TwistStamped.h"
#include "std_msgs/Float64.h"
#include "localization/GpsSpeed.h"

class Observer {
	/*
	 * The observer class is a noise free environment estimator.
	 * It receives GPS & IMU measurements and calculates the location and speed (X,Y,Z,roll,Pitch,yaw, linear speed and angular speed).
	 *
	 */
public:
  /**
   * Constructor:
   */
	Observer();
  /**
   * Destructor:
   */
  virtual ~Observer();
  /**
   * set private variable GPSmeasurement
   */
  void setGPSMeasurement(sensor_msgs::NavSatFix measurement);
  /**
   * set private variable IMUmeasurement
   */
  void setIMUMeasurement(sensor_msgs::Imu measurement);
  /*
  * set the speed measurement that the GPS provides
  */
  void setGPSSpeedMeasurement(localization::GpsSpeed measurement);
  
  /**
   * This function performs the estimation of location.
   */
  void estimator();
  /**
   * return the estimated position
   */
  geometry_msgs::PoseWithCovarianceStamped getEstimatedPose();
  /**
   * return the estimated speed
   */
  geometry_msgs::TwistStamped getEstimatedSpeed();
private:
    geometry_msgs::PoseWithCovarianceStamped estimatedPose;
    geometry_msgs::TwistStamped velocity;
    sensor_msgs::NavSatFix initialGPS, GPSmeasurement;
    sensor_msgs::Imu IMUmeasurement;
    localization::GpsSpeed speedMeasurement;
    bool first_GPS_flag;
    
    /**
    * set GPS coordinates
    */
    void setInitGPS(sensor_msgs::NavSatFix initGPS);
};
