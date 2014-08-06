#ifndef STEREO__H
#define STEREO__H

#include <opencv2/opencv.hpp>
#include "heightmap.h"
#define PI 3.14159

using namespace cv;

Mat getDisparity(Mat left_image, Mat right_image)
{
    cvtColor( left_image,left_image, CV_BGR2GRAY);
    cvtColor( right_image,right_image,CV_BGR2GRAY);
    IplImage temp=left_image;
    IplImage temp2=right_image;
    CvMat *matf= cvCreateMat ( temp.height, temp.width, CV_16S);
    CvStereoBMState * state=cvCreateStereoBMState(CV_STEREO_BM_BASIC,16*4);
    cvFindStereoCorrespondenceBM(&temp,&temp2,matf,state);
    //CvMat * disp_left_visual= cvCreateMat(temp.height, temp.width, CV_8U);
    //cvConvertScale( matf, disp_left_visual, -16 );
    cvNormalize( matf, matf, 0, 255, CV_MINMAX, NULL );
    int i, j;
    uchar *ptr_dst;
    IplImage *cv_image_depth_aux = cvCreateImage (cvGetSize(&temp),IPL_DEPTH_8U, 1);
    for ( i = 0; i < matf->rows; i++)
    {
    	ptr_dst = (uchar*)(cv_image_depth_aux->imageData + i*cv_image_depth_aux->widthStep);
    	for ( j = 0; j < matf->cols; j++ )
    	{
    		ptr_dst[j] = (uchar)((short int*)(matf->data.ptr + matf->step*i))[j];
    		//ptr_dst[3*j+1] = (uchar)((short int*)(matf->data.ptr + matf->step*i))[j];
    		//ptr_dst[3*j+2] = (uchar)((short int*)(matf->data.ptr + matf->step*i))[j];
    	}
    }
    //system("pause");
    //cvSaveImage("disp.ppm", &cv_image_depth_aux);
    //system("pause");
    Mat img(cv_image_depth_aux, true);
	//system("pause");
    //cvReleaseImage(&cv_image_depth_aux);
    cvReleaseMat(&matf);
    //cvReleaseMat(&disp_left_visual);
   // cvReleaseImage(temp);
    //cvReleaseImage(temp2);
    cvReleaseStereoBMState(&state);
	
    //cvtColor(img,img, CV_BGR2GRAY);
    
    return img;
}

Mat filterDisparity(Mat img)
{
  static const int thresh = 55;
  for(int i = 0; i < img.rows; i++)
  {
    for(int j = 0; j < img.cols; j++)
    {
      uchar pxl = img.at<uchar>(i,j);
      //if(pxl > 20) img.at<uchar>(i,j) = 0;
    }
  }
  return img;
}


void ComputeFOVProjection(Mat& result, float fov, float aspect, float zNear, float zFar)
{

    result.at<float>(1,1) = 1 / tan(0.5f * fov);
    result.at<float>(0,0) = result.at<float>(1,1) / aspect;
    result.at<float>(2,2) = (zFar + zNear) / (zNear - zFar);
    result.at<float>(3,2) = 2*(zFar*zNear)/(zNear-zFar);
    result.at<float>(2,3) = -1;
    result.at<float>(3,3) = 0;
}

void ProjectDepthImage(HeightMap* map, Mat img, Vec3D myRight, Vec3D myFront, Vec3D myUp, Vec3D myPos)
{
  static const double fov = PI/8; //45 deg to each side
  double min=100, max=-100;
  
  for(int i = img.rows-1; i >= 0; i--)
  {
    for(int j = 0; j < img.cols; j++)
    {
      uchar depth = img.at<uchar>(i, j);
      if(depth < 10 || depth > 220) continue;
      float depth_m = (256-(int)depth)/15.0f;
      //depth_m = pow(depth, 0.5);
      float right_m = depth_m * sin(fov) * 2*(j - img.cols/2)/img.cols;
      float up_m = depth_m * sin(fov) * 2*(i - img.rows/2)/img.rows;
      Vec3D pos = myPos.add(myFront.multiply(depth_m).add(myRight.multiply(right_m).add(myUp.multiply(up_m))));
      map->setAbsoluteHeightAt((int)(5*pos.x), (int)(5*pos.y), (pos.z));
      if(pos.z > max) max = pos.z;
      if(pos.z < min) min = pos.z;
    }
  }
  printf("min: %f max: %f\n", min, max);
}

void ProjectDepthImage2(HeightMap* map, Mat img, Vec3D myRight, Vec3D myFront, Vec3D myUp, Vec3D myPos)
{
  double min=100, max=-100;
  Mat Q = Mat::eye(4,4,CV_32F);
  ComputeFOVProjection(Q, 1.57079, 1, 5, 10);
  Mat out;
 
  reprojectImageTo3D(img, out, Q); 
  //perspectiveTransform(img, out, Q);
  for(int i = 0; i < img.rows; i++)
    for(int j = 0; j < img.cols; j++)
    {
      /*
      Mat_<float> vec(4,1);
      vec(0)=j; vec(1)=i; 
      //printf("A\n");
      vec(2)=img.at<uchar>(i,j);
      if(vec(2) == 0) continue;
      //printf("B\n");
      vec(3)=1;
      vec = Q*vec;
      vec /= vec(3);
      Vec3D pos = Vec3D(vec(0,0), vec(1,0), vec(2,0));
      */
      
      
      Vec3f pt = out.at<Vec3f>(i,j);
      if(pt[2] < 0) continue;
      Vec3D pos = Vec3D(pt[0], pt[1], pt[2]);
      
      //map->setAbsoluteHeightAt((int)(5*pos.x), (int)(5*pos.y), (pos.z));
      if(pos.z > max) max = pos.z;
      if(pos.z < min) min = pos.z;
      //out.at<Vec3f>(i,j) = Vec3f(0,0, out.at<Vec3f>(i,j)[2]);
    }
  //imshow("disp2x", img);
  //imshow("disp22x", out);
  printf("min: %f max: %f\n", min, max);
}

void ProjectDepthImage3(HeightMap* map, Mat img_disparity, Mat img_rgb, Mat Q, Vec3D myRight, Vec3D myFront, Vec3D myUp, Vec3D myPos)
{
    double Q03 = Q.at<double>(0,3);
    double Q13 = Q.at<double>(1,3);
    double Q23 = Q.at<double>(2,3);
    double Q32 = Q.at<double>(3,2);
    double Q33 = Q.at<double>(3,3);
    double px, py, pz;
    uchar pr, pg, pb;
    for (int i = 0; i < img_disparity.rows; i++)
    {
      //uchar* rgb_ptr = img_rgb.ptr<uchar>(i);
      uchar* disp_ptr = img_disparity.ptr<uchar>(i);
      //printf("row %d\n", i);
      for (int j = 0; j < img_disparity.cols; j++)
      {
	    //Get 3D coordinates
	    uchar d = disp_ptr[j];
	    if ( d == 0 ) continue; //Discard bad pixels
	    double pw = -1.0 * static_cast<double>(d) * Q32 + Q33;
	    px = static_cast<double>(j) + Q03;
	    py = static_cast<double>(i) + Q13;
	    pz = Q23;

	    px = px/pw;
	    py = py/pw;
	    pz = pz/pw;
	    //Insert info into point cloud structure
	    Vec3D point;//pcl::PointXYZRGB point;
	    point.x = px;
	    point.y = py;
	    point.z = pz;
	    
	    //Vec3D pos = myPos.add(myFront.multiply(depth_m).add(myRight.multiply(right_m).add(myUp.multiply(up_m))));
	    map->setAbsoluteHeightAt((int)(point.x), (int)(point.z), (int)(point.y));
	    //uint32_t rgb = (static_cast<uint32_t>(pr) << 16 |
	//		    static_cast<uint32_t>(pg) << 8 | static_cast<uint32_t>(pb));
	 //   point.rgb = *reinterpret_cast<float*>(&rgb);
	  //  point_cloud_ptr->points.push_back (point);
      }
    }
}


Mat handleStereo(Mat left, Mat right)
{
  Mat l,r;
  resize(left, l, Size(left.size().width/2, left.size().height/2), 0, 0, cv::INTER_CUBIC);
  resize(right, r, Size(right.size().width/2, right.size().height/2), 0, 0, cv::INTER_CUBIC);
 
  
  //StereoVar sv();
  //Mat stereo;
  //sv.operator()(l,r,stereo);
  
  Mat stereo = getDisparity(l, r);
  stereo = Mat(stereo, Rect(60, 0, stereo.cols-61, stereo.rows-1));
  //fastNlMeansDenoising(stereo, stereo);
  Mat median;
  medianBlur(stereo, median, 15);
  //stereo = filterDisparity(stereo);
  
  bitwise_and(stereo, median, stereo);
  imshow("stereo", stereo); 
  
  
  //imshow("asdf", median); 
   //imshow("L", l); 
  //imshow("R", r);
  //waitKey(1);
  return stereo;
}



#endif
