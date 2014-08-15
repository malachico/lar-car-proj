
/*
 * RosComm.h
 *
 *  Created on: Thursday, 27. February 2014 12:29PM
 *      Author: autogenerated
 */
#ifndef ROSCOMM_H_
#define ROSCOMM_H_
#include <ros/ros.h>
#include <std_msgs/String.h>
#include <string>       // std::string
#include <iostream>     // std::cout
#include <sstream>
#include <ParameterTypes.h>
#include <tf/tf.h>
#include <boost/thread.hpp>
class ComponentMain;
class RosComm {
  bool _inited;
  ComponentMain   * _comp;
  ros::NodeHandle _nh;
  ros::Publisher _pub_diagnostic;
  boost::thread_group _maintains;
	ros::Subscriber _sub_Location;
	ros::Subscriber _sub_PerVelocity;
	ros::Subscriber _sub_SensorINS;
	ros::Subscriber _sub_SensorGPS;
	ros::Subscriber _sub_SensorCamL;
	ros::Subscriber _sub_SensorCamR;
	ros::Subscriber _sub_SensorWire;
	ros::Subscriber _sub_SensorSICK1;
	ros::Subscriber _sub_SensorSICK2;
	ros::Subscriber _sub_SensorIBEO;
	ros::Subscriber _sub_EffortsTh;
	ros::Subscriber _sub_EffortsSt;
	ros::Subscriber _sub_EffortsJn;
	ros::Subscriber _sub_GpsSpeed;
	ros::Publisher  _pub_GPS;
	ros::Publisher  _pub_INS;
	ros::Publisher  _pub_BladePosition;
	ros::Publisher  _pub_Map;
	ros::Publisher  _pub_MiniMap;
	ros::Publisher  _pub_VOOdometry;
	ros::Publisher  _pub_GpsSpeed;

  bool init(int argc,char** argv);
public:
	RosComm(ComponentMain* comp,int argc,char** argv);
	virtual ~RosComm();
	void LocationCallback(const config::PER::sub::Location::ConstPtr &msg);
	void PerVelocityCallback(const config::PER::sub::PerVelocity::ConstPtr &msg);
	void SensorINSCallback(const config::PER::sub::SensorINS::ConstPtr &msg);
	void SensorGPSCallback(const config::PER::sub::SensorGPS::ConstPtr &msg);
	void SensorCamLCallback(const config::PER::sub::SensorCamL::ConstPtr &msg);
	void SensorCamRCallback(const config::PER::sub::SensorCamR::ConstPtr &msg);
	void SensorWireCallback(const config::PER::sub::SensorWire::ConstPtr &msg);
	void SensorSICK1Callback(const config::PER::sub::SensorSICK1::ConstPtr &msg);
	void SensorSICK2Callback(const config::PER::sub::SensorSICK2::ConstPtr &msg);
	void SensorIBEOCallback(const config::PER::sub::SensorIBEO::ConstPtr &msg);
	void EffortsThCallback(const config::PER::sub::EffortsTh::ConstPtr &msg);
	void EffortsStCallback(const config::PER::sub::EffortsSt::ConstPtr &msg);
	void EffortsJnCallback(const config::PER::sub::EffortsJn::ConstPtr &msg);
	void GpsSpeedCallback(const config::PER::sub::SensorGpsSpeed::ConstPtr &msg);
	void publishGPS( config::PER::pub::GPS &msg);
	void publishINS( config::PER::pub::INS &msg);
	void publishBladePosition( config::PER::pub::BladePosition &msg);
	void publishMap( config::PER::pub::Map &msg);
	void publishMiniMap( config::PER::pub::MiniMap &msg);
	void publishVOOdometry( config::PER::pub::VOOdometry &msg);
	void publishGpsSpeed( config::PER::pub::PerGpsSpeed &msg);

	void publishTransform(const tf::Transform& _tf, std::string srcFrame, std::string distFrame);
	tf::StampedTransform getLastTrasform(std::string srcFrame, std::string distFrame);
	void publishDiagnostic(const diagnostic_msgs::DiagnosticStatus& _report);
	void publishDiagnostic(const std_msgs::Header& header, const diagnostic_msgs::DiagnosticStatus& _report);
	void heartbeat();
};
#endif /* ROSCOMM_H_ */