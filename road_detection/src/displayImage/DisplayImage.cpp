#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <queue>
#include "polyfit.h"

//#include <unistd.h>
//unsigned int microseconds = 5000000;

using namespace std;
using namespace cv;

vector<double> entropyArray;
vector<double>  roadMatrix;

int im_rows,im_cols;
float win_height, win_width;

const float WIN_SIZE = 85.0;

int cut_percents;
int down_percents;

//Mat entropyImage;

/// getROI decleration
Rect getROI(int l, int k);

void printEntropy()
{ 
  for(int i = 0; i < WIN_SIZE*WIN_SIZE; i++) if(entropyArray.at(i)) printf("i: %d - %f\n", i+1, entropyArray.at(i));
}

void printBlockSize()
{
  printf("block size: (%f, %f)\n", win_width, win_height);
}

vector<double> getEntropy(Mat w, int l, int k, int cut_percents, int down_percents)
{
  vector<double> res;
  double Ra,Ga,Ba, Rs,Gs,Bs;
  Vec3b intensity;
  for (int i=0; i < w.rows;i++)
  {
    for(int j=0; j < w.cols;j++)
    {
      
      intensity = w.at<Vec3b>(i,j);
      Ra += intensity[0];
      Ga += intensity[1];
      Ba += intensity[2];
    }
  }
  Ra = Ra/(w.rows*w.cols);
  Ga = Ga/(w.rows*w.cols);
  Ba = Ba/(w.rows*w.cols);
  
  for (int i=0; i < w.rows;i++)
  {
    for(int j=0; j < w.cols;j++)
    {
      
      intensity = w.at<Vec3b>(i,j);
      Rs += abs(intensity[0] - Ra);
      Gs += abs(intensity[1] - Ga);
      Bs += abs(intensity[2] - Ba);
    }
  }
  Rs = Rs/(w.rows*w.cols);
  Gs = Gs/(w.rows*w.cols);
  Bs = Bs/(w.rows*w.cols);
  
  res.push_back(Ra);res.push_back(Ga);res.push_back(Ba);
  res.push_back(Rs);res.push_back(Gs);res.push_back(Bs);
  return res;
}

Mat createEntropyImage(Mat m)
{
  Mat dest = Mat(im_rows, im_cols, CV_8U);
  uchar color;
  
  //*
  Rect roi;
  int l,k;
  for(l=0; l< WIN_SIZE; l++)
  {
    for(k=0; k<WIN_SIZE; k++)
    {	
      roi = getROI(l,k);
      color = entropyArray[l*WIN_SIZE + k];
      dest(roi).setTo(Scalar(color));
      
    }
  }
  //*/
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
  
//   selectedIdxR = findButtomMostRightPointIdx(cont);
  
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
  
  bool goes_down = false; 
    /*
  bool is_down = false;
  selectedIdxL = 0; 
  int minY_idx = (selectedIdxL + direction)%cont_size;
  minY = cont[minY_idx].y;
  /// find left edge of road:
  
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
//     printf("\nprior to getting up\n");
    goUpTheLane(&lastY, &stopLookingForLanes, &goes_up_idx, direction, &cont, &laneI);
//     printf("got up\n");
    
    if(!stopLookingForLanes)
      lanes->push_back(laneI);
    
    ///looking for lane:
    findStartOfLane(selectedIdxR, &goes_up_idx, minY_idx, &cont, direction, &stopLookingForLanes);
//     printf("found lane\n");
    int got_down_idx = goes_up_idx;  
    
     ///look For Next Lane:   
    lookForNextLane(&got_down_idx, direction, &cont);
//     printf("looking for next lane\n");
    
    selectedId("counter: %d\n", counter);xL = got_down_idx;
    counter ++;
    /// assume max amount of lanes is 3 [2 seperating lines]
    if(counter == 3) 
      stopLookingForLanes = true;
    ///repeat:
  }while(!stopLookingForLanes);
  */
  
  /*
  /// find right edge of road:
  height_thresh = 10;
  goes_down = false;
//   printf("looking for right lane\n");
  for(int i= 0-direction; !goes_down; i-=direction)
  {  
    /// TODO: change the condition to be more accurate
    if(cont[i%cont_size].y >= maxY-height_thresh || i%cont_size==0)
    {
//       printf("i:%d, y: %d , stop Y: %d\n", i, cont[i%cont_size].y, maxY-height_thresh);
      goes_down = true;
    }
    else if(i==-1 || i == -2)
    {
      ///somewhy those indexes were corrupted...      
    }
    else 
    {
      LR->push_back(cont[(i%cont_size)]);
    }
  }
//   printf("done looking for lanes\n");

  */
  int PointsToCount = 100;
  int counter = 0;
  goes_down = false;
  for(int i= 0+direction; !goes_down; i+=direction)
  {  
    /// TODO: change the condition to be more accurate
    if(counter == PointsToCount || i%cont_size==0)
    {
      goes_down = true;
    }
    else if(i==-1 || i == -2)
    {
      ///somewhy those indexes were corrupted...      
    }
    else 
    {
      counter++;
      if(cont[(i%cont_size)].x < im_cols/2)
	LL->push_back(cont[(i%cont_size)]);
    }
  }
  
  counter = 0;
  goes_down = false;
  for(int i= 0-direction; !goes_down; i-=direction)
  {  
    /// TODO: change the condition to be more accurate
    if(counter == PointsToCount || i%cont_size==0)
    {
      goes_down = true;
    }
    else if(i==-1 || i == -2)
    {
      ///somewhy those indexes were corrupted...      
    }
    else 
    {
      counter++;
      if(cont[(i%cont_size)].x >im_cols/2)
	LR->push_back(cont[(i%cont_size)]);
    }
  }
}

bool compareEntropies(vector<double> EP, vector<double> EP2) //entropyProperties
{
  double AT = 15; //avarageThreshold
  double VT = 15; // varianceThreshold
  
  if( abs(EP[0]-EP2[0]) > AT || abs(EP[1]-EP2[1]) > AT || abs(EP[2]-EP2[2]) > AT)
    return false;
  else if( abs(EP[3]-EP2[3]) > VT || abs(EP[4]-EP2[4]) > VT || abs(EP[5]-EP2[5]) > VT)
    return false;
  return true;
}

Rect getROI(int l, int k)
{
  int j, i, jPlus, iPlus;
  i = l*win_height;
  j = k*win_width;
    
  if ((i+win_height >= im_rows) && (j+win_width >= im_cols))
  {
    iPlus = im_rows-i-1;
    jPlus = win_width;
  }
  else if(j+win_width >= im_cols)
  {
    jPlus = im_cols-j-1;
    iPlus = win_height;
  }
  else if(i+win_height >= im_rows)
  {
    iPlus = im_rows-i-1;
    jPlus = im_cols-j-1;
  }
  else
  {
    iPlus = win_height;
    jPlus = win_width;
  }
  Rect roi = Rect(j,i,jPlus,iPlus);
  return roi;
}


Mat getROIByLandK(Mat* image, int l, int k)
{
  Rect roi = getROI(l,k);  
  return Mat(*image, roi);
}

struct Pair
{
   int l;
   int k;
   int Fl;
   int Fk;
}; 

bool calcEntropyFor_L_and_k(Mat* m, int l, int k, int fl, int fk)
{
  /// check entropy for this slot: 
  Mat w = getROIByLandK(m, l, k);
  vector<double> ep = getEntropy(w, l, k, cut_percents, down_percents);
  
  Mat fw = getROIByLandK(m, fl, fk);
  vector<double> fep = getEntropy(fw, fl, fk, cut_percents, down_percents);
  
  if(compareEntropies(ep, fep))
  {
   return (entropyArray[l*WIN_SIZE +k] = 255);
  }
  else
    return 0;//(entropyArray[l*WIN_SIZE +k] = 0);
}

bool comparePairs(Pair p, Pair q)
{
  if(p.l == q.l && p.k == q.k)
    return true;
  return false;
}

bool pairVectorContains(vector<Pair> *pairs, Pair p)
{
  for(vector<Pair>::iterator it = pairs->begin(); it != pairs->end(); ++it) 
  {
    if(comparePairs(*it,p))
      return true;
  }
  return false;
}

void findRoadStartFromLK(Mat *image, int l, int k)
{
  vector<Pair> Q;
  
  ///check limits:
  if(l<0 || l == WIN_SIZE)
    return;
  /// mark the vertex as visited
  roadMatrix[l*WIN_SIZE +k] = 1;
  
  Pair p = {l,k,l,k};
  vector<Pair>::iterator it;
  it = Q.begin();
  it = Q.insert(it, p);
  
  Pair t;
  while(!Q.empty())
  {
    t = Q.back();
    Q.pop_back();
    
    if(calcEntropyFor_L_and_k(image, t.l, t.k, t.Fl, t.Fk) )
    {
      /// PROBLEMATIC RECURSION:

      if( t.l>0 && roadMatrix[(t.l-1)*WIN_SIZE +t.k] != 1) // upper neighbor
      {
	roadMatrix[(t.l-1)*WIN_SIZE +t.k] = 1;
	Pair PP = {t.l-1, t.k, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
      if( t.l<WIN_SIZE-1 && roadMatrix[(t.l+1)*WIN_SIZE +t.k] != 1) // lower neighbor
      {
	roadMatrix[(t.l+1)*WIN_SIZE +t.k] = 1;
	Pair PP = {t.l+1, t.k, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
      if(t.k != WIN_SIZE-1 && roadMatrix[t.l*WIN_SIZE +(t.k+1)] != 1) // right neighbor
      {
	roadMatrix[t.l*WIN_SIZE +(t.k+1)] = 1;
	Pair PP = {t.l, t.k+1, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
      if(t.k != 0 && roadMatrix[(t.l)*WIN_SIZE + (t.k-1)] != 1) // left neighbor
      {
	roadMatrix[(t.l)*WIN_SIZE + (t.k-1)] = 1;
	Pair PP = {t.l, t.k-1, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
    }
  }

}


/** *******************************
 *  detectRoad:
 ** *******************************/
vector<double> detectRoad(Mat image, int cut_p, int down_p, int toDebug) 
{  
  vector<double> result;
  if(!image.data )
  {    printf( "No image data \n" );     return result; }
  
  im_rows = image.rows; 
  im_cols = image.cols;
  
  win_height = im_rows/WIN_SIZE;
  win_width  = im_cols/WIN_SIZE;
  
  entropyArray.clear();
  roadMatrix  .clear();
  entropyArray.resize(WIN_SIZE*WIN_SIZE, 0);
  roadMatrix  .resize(WIN_SIZE*WIN_SIZE, 0);
  
  Mat w;
  Rect roi;
  cut_percents = cut_p; 
  down_percents = down_p;
  
  
  findRoadStartFromLK(&image, WIN_SIZE-10, WIN_SIZE*3/4);
  findRoadStartFromLK(&image, WIN_SIZE-10, WIN_SIZE*1/4);
  findRoadStartFromLK(&image, WIN_SIZE-20, WIN_SIZE*2/3);
  findRoadStartFromLK(&image, WIN_SIZE-20, WIN_SIZE*1/3);
  
  Rect ROI = getROI(WIN_SIZE-10, WIN_SIZE*3/4);
  rectangle(image, Point(ROI.x,ROI.y), Point( ROI.x-ROI.width,ROI.y-ROI.height), Scalar(0,0,255),2,8,0);
  
  ROI = getROI(WIN_SIZE-10, WIN_SIZE*1/4);
  rectangle(image, Point(ROI.x,ROI.y), Point( ROI.x-ROI.width,ROI.y-ROI.height), Scalar(0,0,255),2,8,0);
  
  ROI = getROI(WIN_SIZE-20, WIN_SIZE*2/3);
  rectangle(image, Point(ROI.x,ROI.y), Point( ROI.x-ROI.width,ROI.y-ROI.height), Scalar(0,0,255),2,8,0);
  
  ROI = getROI(WIN_SIZE-20, WIN_SIZE*1/3);
  rectangle(image, Point(ROI.x,ROI.y), Point( ROI.x-ROI.width,ROI.y-ROI.height), Scalar(0,0,255),2,8,0);
  
  
///create entropy image  
   Mat entropyImage = createEntropyImage(image);
   
/** *****************************************
 * finding the actual road : 
 * *******************************************/
  
/// eliminate noise on the road: 

  int factor = 2;
  entropyImage = Dilation(entropyImage, factor);
  factor = 2;
  entropyImage = Erosion(entropyImage, factor);

  imshow("debug", entropyImage);
  
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
      //  drawContours(image, contours, k, Scalar(255,255,255), 2, 8);
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
    if(toDebug)
    {
      printf("\nSTART DEBUG\n");
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
	  }
	}
      }
      printf("END DEBUG\n");
    }
  }
  
  if(toDebug)
  {
    imshow("walrus", image);
    waitKey(1);
  }
  
    
  return result;
}


