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
Mat altEntropy;
void printEntropy()
{ 
  for(int i = 0; i < WIN_SIZE*WIN_SIZE; i++) if(entropyArray.at(i)) printf("i: %d - %f\n", i+1, entropyArray.at(i));
}

void printBlockSize()
{
  printf("block size: (%f, %f)\n", win_width, win_height);
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
  for (int i=0; i < im_rows;i++)
  {
    for(int j=0; j < im_cols;j++)
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
int maxY;

int findButtomMostRightPointIdx(vector<Point> cont)
{
  int cont_size = cont.size();
  int minYXR = im_rows+1, selectedIdxR = -1;
  maxY=0;
  
  for(int i=0; i<cont_size; i++)
  {
    if(cont[i].y >= maxY)
    {
      maxY = cont[i].y;
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
  
  return selectedIdxR;
}


int laneHeightThresh = 20;
int idx_thresh = 2;
int height_thresh = 2;
int selectedIdxR;
int minY;

/** *******************************
 *  goUpTheLane:
 ** *******************************/
void goUpTheLane(int *lastY, bool *stopLookingForLanes, int* goes_up_idx, int direction, vector<Point> *cont, vector<Point> * laneI)
{
  bool goes_down = false;
  *lastY = (*cont)[*goes_up_idx].y;
  int cont_size = cont->size();
  for(; !goes_down && !(*stopLookingForLanes); *goes_up_idx += direction)
  {
    if((*cont)[*goes_up_idx%cont_size].y > (*lastY)+height_thresh)
    {
      goes_down = true;
    }
    else 
    {
      *lastY = (*cont)[*goes_up_idx%cont_size].y;
      laneI->push_back((*cont)[(*goes_up_idx)%cont_size]);
    }
    if((*goes_up_idx)%cont_size >= selectedIdxR-idx_thresh && (*goes_up_idx)%cont_size <= selectedIdxR+idx_thresh)
    {
      *stopLookingForLanes = true;
      break;
    }
  }
}

/** *******************************
 *  findStartOfLane:
 ** *******************************/
void findStartOfLane(int selectedIdxR, int* goes_up_idx, int minY_idx,
		     vector<Point> *cont, int direction, bool *stopLookingForLanes)
{
    bool goes_up = false;
    int cont_size = cont->size();
    
  ///looking for big change in height:
    for(*goes_up_idx=minY_idx; !goes_up;)
    {
      if(minY-(*cont)[*goes_up_idx%cont_size].y >= laneHeightThresh)
	goes_up = true;
      else
      {
	*goes_up_idx = (*goes_up_idx+direction)%cont_size;
      }
      if (*goes_up_idx%cont_size >= selectedIdxR-idx_thresh 
       && *goes_up_idx%cont_size <= selectedIdxR+idx_thresh)
      {
	*stopLookingForLanes = true;
	break;
      }
    }
}

/** *******************************
 *  lookForNextLane:
 ** *******************************/
void lookForNextLane(int *got_down_idx, int direction, vector<Point> *cont)
{
  bool got_down = false;
  int cont_size = cont->size();
  
  for(; !got_down; *got_down_idx += direction)
  {
    if((*cont)[(*got_down_idx)%cont_size].y == minY)
    {
      got_down = true;
    }
  }
}


/** *******************************
 *  findLimits :
 ** *******************************/

void findLimits(vector<Point> *LR, vector<Point> *LL, vector<vector<Point> > *lanes, vector<Point> cont)
{
  int cont_size = cont.size();
  
  selectedIdxR = findButtomMostRightPointIdx(cont);
  
  int selectedIdxL = 0;
  int direction = 0,counts = 1;
  for(; direction == 0 ; counts++)
  {
    if(cont[(selectedIdxL+counts)%cont_size].y > cont[selectedIdxL].y)
    {
      direction = 1;
    }
    else if(cont[(selectedIdxL+counts)%cont_size].x > cont[selectedIdxL].y)
    {
      direction = -1;
    }
  }
  int minY_idx = (selectedIdxL + direction)%cont_size;
  minY = cont[minY_idx].y;
  bool is_down = false;
  selectedIdxL = 0;
  
  /// find left edge of road:
  bool goes_down = false; 
  int lastY=cont[selectedIdxL].y;
  
  int ii;
  for( ii=selectedIdxL; !goes_down; ii+=direction)
  {
    if(cont[ii%cont_size].y == maxY)
    {
      goes_down = true;
    }
    else 
    {
      lastY = cont[ii%cont_size].y;
      LL->push_back(cont[ii%cont_size]); 
    }
  }
  selectedIdxL = ii%cont_size;
  
  bool stopLookingForLanes = false;
  int counter =0;
  do 
  {
    vector<Point> laneI;
    height_thresh = 5;
    int goes_up_idx = selectedIdxL;
    
    ///going up the lane:    
    goUpTheLane(&lastY, &stopLookingForLanes, &goes_up_idx, direction, &cont, &laneI);
    
    if(!stopLookingForLanes)
      lanes->push_back(laneI);
    
    ///looking for lane:
    findStartOfLane(selectedIdxR, &goes_up_idx, minY_idx, &cont, direction, &stopLookingForLanes);
    
    int got_down_idx = goes_up_idx;  
    
     ///look For Next Lane:   
    lookForNextLane(&got_down_idx, direction, &cont);
    
    
    selectedIdxL = got_down_idx;
    counter ++;
    /// assume max amount of lanes is 4 [3 seperating lines]
    if(counter == 3) 
      stopLookingForLanes = true;
    ///repeat:
  }while(!stopLookingForLanes);
  
  /// find right edge of road:
  height_thresh = 10;
  goes_down = false;
  
  for(int i= 0-direction; !goes_down; i-=direction)
  {  
    if(cont[i%cont_size].y >= maxY-height_thresh)
    {
//       printf("i:%d, y: %d , stop Y: %d\n", i, cont[i%cont_size].y, maxY-height_thresh);
      goes_down = true;
    }
    else if(i==-1 || i == -2)
    {}
    else 
    {
      LR->push_back(cont[(i%cont_size)]);
    }
  }
}


/** *******************************
 *  detectRoad:
 ** *******************************/
vector<double> detectRoad(Mat image, int cut_p, int down_p) 
{
  vector<double> result;
  if(!image.data )
  {    printf( "No image data \n" );     return result; }

   
  ///calculate the probability of grayscale Gi to show at the image
  
  
  im_rows = image.rows; 
  im_cols = image.cols;
  
  {
    //printf("%d, %d", im_rows, im_cols);
  }
  
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
    /// take the big ones:
    if( (contourArea(contours[k]) > 10000 && hierarchy[k][3] == -1) )
    {
      selected_contours.push_back(contours[k]);
      selected_contour = k;
      /// to remove [debug purposes]:
        //drawContours(image, contours, k, Scalar(0,0,0), 2, 8);
    }
    
  }
  
  int s = selected_contours.size();
  
  for(int i=0; i<s; i++)
  {    
    vector<Point> limitsR;
    vector<Point> limitsL;
    vector<vector<Point> > lanes;
    findLimits(&limitsR, &limitsL, &lanes, selected_contours[i]);
    
    double a[3];
    polyfit(a, limitsL, limitsL.size()-1);
    
    double b[3];
    polyfit(b, limitsR, limitsR.size()-1);
    
    vector< double*> Cs;
    for(int j=0; j<lanes.size(); j++)
    {
      double c[3];
      polyfit(c, lanes[j], lanes[j].size());
      Cs.push_back(c);
    }
    
    if(a[0] == a[0] && a[1] == a[1] && a[2] == a[2])
    {
      result.push_back(limitsL[0].y);
      result.push_back(limitsL[limitsL.size()-1].y);
      result.push_back(a[2]);
      result.push_back(a[1]);
      result.push_back(a[0]);
    }
    else
    {
      result.push_back(0);
      result.push_back(0);      
      result.push_back(0);
      result.push_back(0);
      result.push_back(0);
    }
    
    for(int j=0; j<Cs.size();j++)
    {
      if(Cs[j][0] == Cs[j][0] && Cs[j][1] == Cs[j][1] && Cs[j][2] == Cs[j][2])
      {
	result.push_back(lanes[j][0].y);
	result.push_back(lanes[j][lanes[j].size()-1].y);
	result.push_back(Cs[j][2]);
	result.push_back(Cs[j][1]);
	result.push_back(Cs[j][0]);
      }
      else
      {
	result.push_back(0);
	result.push_back(0);
	result.push_back(0);
	result.push_back(0);
	result.push_back(0);
      } 
    }
    
    if(b[0] == b[0] && b[1] == b[1] && b[2] == b[2])
    {
      result.push_back(limitsR[0].y);
      result.push_back(limitsR[limitsR.size()-1].y); 
      result.push_back(b[2]);
      result.push_back(b[1]);
      result.push_back(b[0]);
    }
    else
    {
      result.push_back(0);
      result.push_back(0);      
      result.push_back(0);
      result.push_back(0);
      result.push_back(0);
    }
    //printBlockSize();
    //* debug purposes: 
    printf("\nSTART DEBUG\n");
    printf("right edge size: %d\n", limitsR.size());
    if(selected_contours.size() > 0)
      circle(image, selected_contours[0][0], 8, Scalar(255,255,255), -1,8,0);
    for(int j=0; j<limitsL.size(); j++)
    {
      int x = limitsL[j].y;
      double y = a[0] + a[1]*x*1.0 + a[2]*x*x*1.0;
      
      circle(image, Point(y,x), 3, Scalar(0,0,255), -1, 8, 0);
      //circle(image, limitsL[j], 3, Scalar(0,255,255), -1, 8, 0);
    }
    
    for(int j=0; j<limitsR.size() && j < 500; j++)
    {
      int x = limitsR[j].y;
      double y = b[0] + b[1]*x*1.0 + b[2]*x*x*1.0;
      
      circle(image, Point(y,x),3, Scalar(255,0,0), -1, 8, 0);
//       circle(image, limitsR[j], 3, Scalar(255,0,255), -1, 8, 0);
    }
    
    for(int j=0; j<lanes.size(); j++)
    {
      for(int k=0; k<lanes[j].size(); k++)
      {
	if( &lanes[j][k] != 0)
	{
	  int x = lanes[j][k].y;
	  double y = Cs[j][0] + Cs[j][1]*x*1.0 + Cs[j][2]*x*x*1.0;
	  
	  circle(image, Point(y,x), 3, Scalar(120*(j+1),120*(j+1),0), -1, 8, 0);
	  //circle(image, lanes[j][k], 3, Scalar(0,60*(j+1),0), -1, 8, 0);
	}
      }
    }
    printf("END DEBUG\n");
    //*/
  }
  
   imshow("walrus", image);
   waitKey(1);
  
    
  return result;
}













