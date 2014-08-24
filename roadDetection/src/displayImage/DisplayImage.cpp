#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>

#include "polyfit.h"

//#include <unistd.h>
//unsigned int microseconds = 5000000;

using namespace std;
using namespace cv;

vector<double> entropyArray;
int im_rows,im_cols;
float win_height, win_width;
unsigned int filterValues[] = {0,255};


const float WIN_SIZE = 250.0;

int cut_percents;
int down_percents;

Mat entropyImage;

void printEntropy()
{ 
  for(int i = 0; i < WIN_SIZE*WIN_SIZE; i++) if(entropyArray.at(i)) printf("i: %d - %f\n", i+1, entropyArray.at(i));
}

void calcEntropy(Mat w, int l, int k, int cut_percents, int down_percents)
{
  Vec3b intensity;
  double Pgi;
  
  /// if the pixel is in the cut percents area - paint it black
  if((l < (im_rows/win_height)*(cut_percents/100.0))) 
  {
    entropyArray.at(l*WIN_SIZE +k) = 0;
    return;
  }
  if((l > (im_rows/win_height)*(down_percents/100.0))) 
  {
    entropyArray.at(l*WIN_SIZE +k) = 0;
    return;
  }
  
  
  /// else -
  int threshold = 20;
  int greyThreshold = 50;
  double Ra, Ga, Ba;
  uchar Rm=0, Gm=0, Bm=0;
  
  for (int i=0; i < w.rows;i++)
  {
    for(int j=0; j < w.cols;j++)
    {
      
      intensity = w.at<Vec3b>(i,j);
      Ra += intensity[0];
      Ga += intensity[1];
      Ba += intensity[2];
      
      if(intensity[0]>Rm)
	  Rm = intensity[0];
      if(intensity[1]>Gm)
	  Gm = intensity[1];
      if(intensity[2]>Bm)
	  Bm = intensity[2];
    }
  }
  Ra = Ra/(w.rows*w.cols);
  Ga = Ga/(w.rows*w.cols);
  Ba = Ba/(w.rows*w.cols);
  
  if( abs(Rm-Ra) >= threshold || abs(Bm-Ba) >= threshold || abs(Bm-Ba) >= threshold )
    entropyArray.at(l*WIN_SIZE +k) = 0;
  else if( abs(Ga-Ra) >= greyThreshold || abs(Ga-Ba) >= greyThreshold || abs(Ra-Ba) >= greyThreshold)
  {
    entropyArray.at(l*WIN_SIZE +k) = 0;
  }
  else
    entropyArray.at(l*WIN_SIZE +k) = 255;
}


Mat createEntropyImage()
{
  Mat dest = Mat(im_rows, im_cols, CV_8U);
  uchar color;
  for (int i=0; i < dest.rows;i++)
  {
    for(int j=0; j < dest.cols;j++)
    {
	color = (uchar)(entropyArray.at(((int)((i/win_height)))*WIN_SIZE +(int)(j/win_width)));
	
	dest.at<uchar>(i,j) = color;
    } 
  }
  return dest;
}



/** *********************
	Erode
 ** *********************/

Mat Erosion(Mat src, double factor)
{
  Mat erosion_dst;
//   int erosion_type = MORPH_ELLIPSE;
  int erosion_type = MORPH_RECT;
  Mat element = getStructuringElement( erosion_type,
                                       Size( factor*win_width+1, factor*win_height +1),
                                       Point( win_width, win_height ) );

  /// Apply the erosion operation
  erode( src, erosion_dst, element );
  return erosion_dst;
}


/** *********************
	Dilate
 ** *********************/

Mat Dilation(Mat src, double factor)
{
  Mat Dilation_dst;
//   int dilation_type = MORPH_ELLIPSE;
  int dilation_type = MORPH_RECT;
  
  Mat element = getStructuringElement( dilation_type,
                                       Size( factor*win_width+1, factor*win_height +1),
                                       Point( win_width, win_height ) );

  /// Apply the Dilation operation
  dilate( src, Dilation_dst, element );
  return Dilation_dst;
}

/** *******************************
 *  get indexes for limits:
 *  findDownEdges:
 ** *******************************/
vector<int> findBottomEdges(vector<Point> cont)
{
  int cont_size = cont.size();
  int minY=5000 , maxY=0, minYXL=5000, selectedIdxL=-1, minYXR = 5000, selectedIdxR=-1;
  int minXYL = 10000, minXYR = 0, alt_idx_l=-1, alt_idx_r=-1;
  for(int i=0; i<cont_size; i++)
  {
    if(cont[i].y >= maxY)
    {
      maxY = cont[i].y;
      if(cont[i].x < minXYL)
      {
	minXYL = cont[i].x;
	alt_idx_l = i;
      }
      if(cont[i].x > minXYR)
      {
	minXYR = cont[i].x;
	alt_idx_r = i;
      }
    }
    if(cont[i].y < minY)
    {
      minY = cont[i].y;
    }
    if(cont[i].x <= 1)
    {
      if(cont[i].y < minYXL)
      {
	minYXL = cont[i].y;
	selectedIdxL = i;
      }
    }
    if(cont[i].x >= im_cols-2)
    {
      if(cont[i].y < minYXR)
      {
	minYXR = cont[i].y;
	selectedIdxR = i;
      }
    }
  }
  vector<int> result;
  result.push_back(selectedIdxL);
  result.push_back(selectedIdxR);
  result.push_back(alt_idx_l);
  result.push_back(alt_idx_r);
  return result;
}

/** *******************************
 *  findLimits:
 ** *******************************/

int pix_threshold = 1;
void findLimits(Mat *m4, vector<Point> *LR, vector<Point> *LL, vector<vector<Point> > *lanes, vector<Point> cont)
{
  int cont_size = cont.size();
  vector<int> edges = findBottomEdges(cont);
  int selectedIdxL = edges[0];
  int selectedIdxR = edges[1];
  int alt_idx_l = edges[2];
  int alt_idx_r = edges[3];
    
  int direction = 0,counts = 1;
  for(; direction == 0 ; counts++)
  {
    if(cont[(selectedIdxL+counts)%cont_size].y > cont[selectedIdxL].y)
    {
      direction = 1;
    }
    else if(cont[(selectedIdxL+counts)%cont_size].y < cont[selectedIdxL].y)
    {
      direction = -1;
    }
  }
  bool goes_down = false; 
  int height_thresh = 2;
  if(selectedIdxL == -1)
    selectedIdxL = alt_idx_l;
  int lastY=cont[selectedIdxL].y;
  for(int i=selectedIdxL; !goes_down; i-=direction)
  {
    if(cont[i%cont_size].y > lastY+height_thresh)
    {
      goes_down = true;
    }
    else 
    {
      lastY = cont[i%cont_size].y;
      LL->push_back(cont[i%cont_size]); 
    }
  }
  //* find lanes:
  int counter =0;
  bool stopLookingForLanes = false;
  int idx_thresh = 2;
  do 
  {
  vector<Point> laneI;
  bool goes_up = false;
  int minY_idx = (selectedIdxL + direction)%cont_size;
  int minY = cont[minY_idx].y;
  int laneHeightThresh = 20;
  int goes_up_idx;
  height_thresh = 10;
  
  ///looking for big change in height:
  for(goes_up_idx=minY_idx; !goes_up;)
  {
    if(minY-cont[goes_up_idx%cont_size].y >= laneHeightThresh)
      goes_up = true;
    else
    {
      goes_up_idx = (goes_up_idx+direction)%cont_size;
    }
    if(goes_up_idx%cont_size >= selectedIdxR-idx_thresh && goes_up_idx%cont_size <= selectedIdxR+idx_thresh)
    {
      stopLookingForLanes = true;
      break;
    }
  }
  ///going up the lane:
  goes_down = false;
  lastY=cont[goes_up_idx].y;
  for(; !goes_down && !stopLookingForLanes; goes_up_idx += direction)
  {
    if(cont[goes_up_idx%cont_size].y > lastY+height_thresh)
    {
      goes_down = true;
    }
    else 
    {
      lastY = cont[goes_up_idx%cont_size].y;
      laneI.push_back(cont[goes_up_idx%cont_size]);
    }
    if(goes_up_idx%cont_size >= selectedIdxR-idx_thresh && goes_up_idx%cont_size <= selectedIdxR+idx_thresh)
    {
      stopLookingForLanes = true;
      break;
    }
  }
  ///going down the lane:
  if(!stopLookingForLanes)
    lanes->push_back(laneI);
  bool got_down = false;
  int got_down_idx = goes_up_idx;
  for(; !got_down; got_down_idx += direction)
  {
     if(cont[got_down_idx%cont_size].y == minY)
    {
      got_down = true;
    }
  }
  selectedIdxL = got_down_idx;
  counter ++;
  if(counter == 3)
    stopLookingForLanes = true;
  ///repeat:
  }while(!stopLookingForLanes);
  
  
  // untill here...*/
  height_thresh = 2;
  goes_down = false;
  if(selectedIdxR == -1)
    selectedIdxR = alt_idx_r;
  lastY=cont[selectedIdxR].y;
  for(int i=selectedIdxR; !goes_down; i+=direction)
  {
    if(cont[i%cont_size].y > lastY+height_thresh)
    {
      goes_down = true;
    }
    else 
    {
      lastY = cont[i%cont_size].y;
      LR->push_back(cont[i%cont_size]);
    }
  }
}

/** *******************************
 *  detectRoad:
 ** *******************************/
int detectRoad(Mat image, int cut_p, int down_p) 
{
  if(!image.data )
  {    printf( "No image data \n" );     return -1; }
  
  ///calculate the probability of grayscale Gi to show at the image
  
  
  im_rows = image.rows; 
  im_cols = image.cols;
  
  win_height = im_rows/WIN_SIZE;
  win_width  = im_cols/WIN_SIZE;
  
  entropyArray.resize(WIN_SIZE*WIN_SIZE, 0);

  Mat w;
  Rect roi;
  cut_percents = cut_p; 
  down_percents = down_p;

  
  for(int i=0; i<im_rows; i+=win_height)
  {
    for(int j=0; j<im_cols; j+=win_width)
    {
      ///-> extract window out of the image:
      
      if((i+win_height >= im_rows) && !(j+win_width >= im_cols)) 
	roi =  Rect(j,i,win_width,im_rows-i-1);
      
      else if(!(i+win_height >= im_rows) && (j+win_width >= im_cols)) 
	roi =  Rect(j,i,im_cols-j-1,win_height);
      
      else if ((i+win_height >= im_rows) && (j+win_width >= im_cols))
	roi =  Rect(j,i,im_cols-j-1,im_rows-i-1);
      
      else
	roi =  Rect(j,i,win_width,win_height);	

///    calc entropy for each window.calculation:
      w = Mat(image, roi);
      calcEntropy(w, i/win_height, j/win_width, cut_percents, down_percents);
    }
  }
  
///create entropy image  
   entropyImage = createEntropyImage();
  
/** *****************************************
 * finding the actual road : 
 * *******************************************/
  
/// eliminate noise on the road:
  int factor = 3;
  entropyImage = Erosion(entropyImage, factor);
  factor = 4;
  entropyImage = Dilation(entropyImage, factor);
  imshow("ely", entropyImage);
  
/// finding clusters:
  vector<vector<Point> > contours;
  vector<vector<Point> > selected_contours;
  vector<Vec4i> hierarchy;  
  
  findContours( entropyImage, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );

  int largest=0, second;
  double max_area=0, second_area=0, curr_area=0;
  int selected_contour;
  
  for( int k = 0; k < contours.size(); k++ )
  {
    curr_area = contourArea(contours[k]);
    
    if(curr_area > max_area)
    {
      second = largest;
      second_area = max_area;
      
      max_area = curr_area;      
      largest = k;
    }
    else if( curr_area > second_area)
    {
      second_area = curr_area;
      second = k;
    }
    if( (contourArea(contours[k]) > 10000 && hierarchy[k][3] == -1) )
    {
      selected_contours.push_back(contours[k]);
      selected_contour = k;
      /// to remove:
       drawContours(image, contours, k, Scalar(0,0,0), 2, 8);
    }
    
  }
  
  int towrite = 0;
  int s = selected_contours.size();

  
  for(int i=0; i<s; i++)
  {    
    vector<Point> limitsR;
    vector<Point> limitsL;
    vector<vector<Point> > lanes;
    findLimits(&image, &limitsR, &limitsL, &lanes, selected_contours[i]);
    
    double a[3];
    polyfit(a, limitsL, limitsL.size()-10);
    printf("left edge: %f x^2 + %f x + %f\n", a[2], a[1], a[0]);
    double b[3];
    polyfit(b, limitsR, limitsR.size()-10);
    printf("right edge: %f x^2 + %f x + %f\n\n", b[2], b[1], b[0]);
    
    vector< double*> Cs;
    for(int j=0; j<lanes.size(); j++)
    {
      double c[3];
      polyfit(c, lanes[j], lanes[j].size());
      Cs.push_back(c);
//       printf("first lane: %f x^2 + %f x + %f\n\n", c[2], c[1], c[0]);
    }
        
    //* debug purposes:
    for(int j=0; j<limitsL.size()-2; j++)
    {
      int x = limitsL[j].y;
      double y = a[0] + a[1]*x*1.0 + a[2]*x*x*1.0;
      
      circle(image, Point(y,x), 3, Scalar(0,0,255), -1, 8, 0);
      //circle(image, limitsL[j], 3, Scalar(0,255,255), -1, 8, 0);
    }
    for(int j=0; j<limitsR.size()-2; j++)
    {
      int x = limitsR[j].y;
      double y = b[0] + b[1]*x*1.0 + b[2]*x*x*1.0;
      
      circle(image, Point(y,x),3, Scalar(255,0,0), -1, 8, 0);
      //circle(image, limitsR[j], 3, Scalar(255,255,0), -1, 8, 0);
    }
    //printf("lanes size: %d\n", lanes.size());
    for(int j=0; j<lanes.size(); j++)
    {
      for(int k=0; k<lanes[j].size()-2; k++)
      {
	int x = lanes[j][k].y;
	double y = Cs[j][0] + Cs[j][1]*x*1.0 + Cs[j][2]*x*x*1.0;
	
	circle(image, Point(y,x), 3, Scalar(120*(j+1),120*(j+1),0), -1, 8, 0);
	//circle(image, lanes[j][k], 3, Scalar(0,60*(j+1),0), -1, 8, 0);
      }
    }
    //*/
  }
  
  imshow("walrus", image);
  waitKey(0);
  
  return 0;
}













