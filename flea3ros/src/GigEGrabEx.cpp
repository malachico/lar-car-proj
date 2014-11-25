// FLEA 3 ROS 		By:Roman Kudinov

//

#include "FlyCapture2.h"
#include <ros/ros.h>
#include <ros/spinner.h>
#include <sensor_msgs/image_encodings.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CompressedImage.h>
#include <cv_bridge/cv_bridge.h>
#include <boost/thread/thread.hpp>
#include <opencv2/opencv.hpp>
#include <cv_bridge/cv_bridge.h>

#include <image_transport/image_transport.h>


using namespace FlyCapture2;

image_transport::Publisher publishers[10];

// PGRGuid cameraMasterSlave[2];
// ros::Publisher publishers[10];

// sensor_msgs::CompressedImage CompressMsg(sensor_msgs::Image& message)
// {
//   sensor_msgs::CompressedImage compressed;
//   compressed.header = message.header;
//   compressed.format = message.encoding;
//   compressed.format += "; png compressed";
//  
//   cv_bridge::CvImagePtr cv_ptr;
//   try
//   {
//     cv_ptr = cv_bridge::toCvCopy(message, "rgb8");
//     // Compress image
//     std::vector<int> params;
//     params.resize(3, 0);
//     params[0] = CV_IMWRITE_PNG_COMPRESSION;
//     params[1] = 2;
//     if (cv::imencode(".png", cv_ptr->image, compressed.data, params))
//     {
//       //float cRatio = (float)(cv_ptr->image.rows * cv_ptr->image.cols * cv_ptr->image.elemSize())
//       /// (float)compressed.data.size();
//       //ROS_DEBUG("Compressed Image Transport - Codec: png, Compression: 1:%.2f (%lu bytes)", cRatio, compressed.data.size());
//     }
//     else
//     {
//       ROS_ERROR("cv::imencode (png) failed on input image");
//     }
//   }
//   catch (cv_bridge::Exception& e)
//   {
//     ROS_ERROR("%s", e.what());
//   }
//   catch (cv::Exception& e)
//   {
//   ROS_ERROR("%s", e.what());
//   }
//   
//   //int bitDepth = enc::bitDepth(message.encoding);
//   //int numChannels = enc::numChannels(message.encoding);
//   return compressed;
// }

void PrintError( Error error )
{
    error.PrintErrorTrace();
}

// void printRegister(GigECamera *cam)
// {
//   unsigned int regVal = 0;
//   Error error;
//   error = cam->ReadRegister( 0x11f8, &regVal );
//   if (error != PGRERROR_OK)
//   {
//       PrintError( error );
//       return;
//   }
//   else
//     printf("0x11f8  %x\n",regVal);
//   error = cam->ReadRegister( 0x1104, &regVal );
//   if (error != PGRERROR_OK)
//   {
//       PrintError( error );
//       return;
//   }
//   else
//     printf("0x1104  %x\n",regVal);
//   error = cam->ReadRegister( 0x1508, &regVal );
//   if (error != PGRERROR_OK)
//   {
//       PrintError( error );
//       return;
//   }
//   else
//     printf("0x1508  %x\n",regVal);
// }
// 
// void setCamMaster(GigECamera *cam)
// {
//   
//   printf("Setting camera as master\n");
//   printf("before registery setup\n");
//   printRegister(cam);
//   cam->WriteRegister(0x11f8,0x60000000);
//   cam->WriteRegister(0x1104,0x40000030);
//   cam->WriteRegister(0x1508,0x83000400);
//   cam->WriteRegister(0x083c,0xc20004f0);
//   cam->WriteRegister(0x081c,0xc200078d);
//   printf("after registery setup\n");
//   printRegister(cam);
// 
// }
// 
// void setCamSlave(GigECamera *cam)
// {
//   printf("Setting camera as slave\n");
//   printf("before registery setup\n");
//   printRegister(cam);
//   cam->WriteRegister(0x11f8,0x40000000);
//   cam->WriteRegister(0x0830,0x835e0000);
//   cam->WriteRegister(0x0834,0xc2000021);
//   cam->WriteRegister(0x081c,0xc200078d);
//   printf("after registery setup\n");
//   printRegister(cam);
// }

int RunSingleCamera(PGRGuid guid, int id)
{
    Error error;
    GigECamera cam;
    //GigECamera cam;
    
    

    printf( "Connecting to camera...\n" );

    // Connect to a camera
    error = cam.Connect(&guid);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    // Get the camera information
    CameraInfo camInfo;
    error = cam.GetCameraInfo(&camInfo);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    unsigned int numStreamChannels = 0;
    error = cam.GetNumStreamChannels( &numStreamChannels );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    for (unsigned int i=0; i < numStreamChannels; i++)
    {
        GigEStreamChannel streamChannel;
        error = cam.GetGigEStreamChannelInfo( i, &streamChannel );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        printf( "\nPrinting stream channel information for channel %u:\n", i );
        //PrintStreamChannelInfo( &streamChannel );
    }    

    printf( "Querying GigE image setting information...\n" );

    GigEImageSettingsInfo imageSettingsInfo;
    error = cam.GetGigEImageSettingsInfo( &imageSettingsInfo );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    GigEImageSettings imageSettings;
    imageSettings.offsetX = 0;
    imageSettings.offsetY = 0;
    imageSettings.height = imageSettingsInfo.maxHeight;
    imageSettings.width = imageSettingsInfo.maxWidth;
    imageSettings.pixelFormat = PIXEL_FORMAT_RAW8;

    printf( "Setting GigE image settings...\n" );

    error = cam.SetGigEImageSettings( &imageSettings );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
//     if (cameraMasterSlave[0] == guid)
//       setCamMaster(&cam);
//     else if (cameraMasterSlave[1] == guid)
//       setCamSlave(&cam);
    printf( "Starting image capture...\n" );

    // Start capturing images
    error = cam.StartCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    Image rawImage;  
    int seq = 0;
    while(1)
    {              
        // Retrieve an image
        error = cam.RetrieveBuffer( &rawImage );
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            continue;
        }
	
	// Copy the data into an OpenCV Mat structure
	cv::Mat bayer8BitMat(rawImage.GetRows(), rawImage.GetCols(), CV_8UC1, rawImage.GetData());
	cv::Mat rgb8BitMat(rawImage.GetRows(), rawImage.GetCols(), CV_8UC3);
	cv::cvtColor(bayer8BitMat, rgb8BitMat, CV_BayerGR2RGB);
	sensor_msgs::ImagePtr msg = cv_bridge::CvImage(std_msgs::Header(), "bgr8", rgb8BitMat).toImageMsg();
	msg->header.seq = seq;
	msg->header.frame_id = "1";
	msg->header.stamp = ros::Time::now();
        msg->height = rawImage.GetRows();
	msg->width = rawImage.GetCols();
	msg->encoding = sensor_msgs::image_encodings::RGB8;
	msg->is_bigendian = 0;
	msg->step = rawImage.GetStride();

	publishers[id].publish(msg);//CompressMsg(msg));
        seq++;
    }         

    printf( "Stopping capture...\n" );

    // Stop capturing images
    error = cam.StopCapture();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }      

    // Disconnect the camera
    error = cam.Disconnect();
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    return 0;
}

int main(int argc, char** argv)
{    
    ros::init(argc, argv, "flea3ros");
    ros::NodeHandle n;
    image_transport::ImageTransport it(n);
    
    Error error;
    BusManager busMgr;
    CameraInfo camInfo[10];
    unsigned int numCamInfo = 10;
    error = BusManager::DiscoverGigECameras( camInfo, &numCamInfo );
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }

    unsigned int numCameras;
    error = busMgr.GetNumOfCameras(&numCameras);
    if (error != PGRERROR_OK)
    {
        PrintError( error );
        return -1;
    }
    for (unsigned int i=0; i < numCameras; i++)
    {
	char topicName[90];
	
	sprintf(topicName, "SENSORS/FLEA3/%d", i);
// 	publishers[i] = n.advertise<sensor_msgs::Image>(topicName, 10);
	publishers[i] = it.advertise(topicName, 10);
        
	PGRGuid guid;
        error = busMgr.GetCameraFromIndex(i, &guid);
        if (error != PGRERROR_OK)
        {
            PrintError( error );
            return -1;
        }

        InterfaceType interfaceType;
        error = busMgr.GetInterfaceTypeFromGuid( &guid, &interfaceType );
        if ( error != PGRERROR_OK )
        {
            PrintError( error );
            return -1;
        }
//         if (i < 2)
// 	  cameraMasterSlave[i] = guid;
        if ( interfaceType == INTERFACE_GIGE)
	    boost::thread t(RunSingleCamera, guid, i);
	
        
    }

    
    ros::AsyncSpinner spinner(4); // Use 4 threads
    spinner.start();
    ros::waitForShutdown();

    return 0;
}
