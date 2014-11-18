#include <opencv2/opencv.hpp>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <stdexcept>


//#include <unistd.h>
//unsigned int microseconds = 5000000;

using namespace std;
using namespace cv;

vector<int>  connectivityMatrix, coverageMatrix;

int im_rows,im_cols;

float win_height, win_width;

const float WIN_SIZE = 85.0;

int cut_percents;
int down_percents;
Mat entropyImage;

/** new variables **/
int level;
bool finished;
/** end **/



void printConnectivityMatrix()
{
  printf("\n");
  for(int i=0; i<WIN_SIZE; i++)
  {
    for(int j=0; j<WIN_SIZE; j++)
      printf("%d,", connectivityMatrix[i*WIN_SIZE +j] );
    printf("\n");
  }
}

/// until here


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
    return (connectivityMatrix[l*WIN_SIZE +k] = level);
  else
    return(connectivityMatrix[l*WIN_SIZE +k] = 0);
}

void findConnectivityComponentsStartFromLK(Mat *image, int l, int k)
{
  vector<Pair> Q;
  
  ///check limits:
  if(l<0 || l == WIN_SIZE)
    return;
  /// mark the vertex as visited
  coverageMatrix[l*WIN_SIZE +k] = 1;
  
  Pair p = {l,k,l,k};
  vector<Pair>::iterator it;
  it = Q.begin();
  it = Q.insert(it, p);
  
  Pair t;
  int i=0;
  while(!Q.empty())
  {
    t = Q.back();
    Q.pop_back();
    
    if(calcEntropyFor_L_and_k(image, t.l, t.k, t.Fl, t.Fk) )
    {      
      if( t.l>0 && coverageMatrix[(t.l-1)*WIN_SIZE +t.k] == 0) // upper neighbor
      {
	coverageMatrix[(t.l-1)*WIN_SIZE +t.k] = 1;
	Pair PP = {t.l-1, t.k, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
      if( t.l<WIN_SIZE-1 && coverageMatrix[(t.l+1)*WIN_SIZE +t.k] == 0) // lower neighbor
      {
	coverageMatrix[(t.l+1)*WIN_SIZE +t.k] = 1;
	Pair PP = {t.l+1, t.k, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
      if(t.k != WIN_SIZE-1 && coverageMatrix[t.l*WIN_SIZE +(t.k+1)] == 0) // right neighbor
      {
	coverageMatrix[t.l*WIN_SIZE +(t.k+1)] = 1;
	Pair PP = {t.l, t.k+1, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
      if(t.k != 0 && coverageMatrix[(t.l)*WIN_SIZE + (t.k-1)] == 0) // left neighbor
      {
	coverageMatrix[(t.l)*WIN_SIZE + (t.k-1)] = 1;
	Pair PP = {t.l, t.k-1, t.l, t.k};
	it = Q.begin();
	it = Q.insert(it, PP);
      }
    }
  }

}


Pair getEmptySlot()
{
  Pair res = {-1,-1,0,0};
  finished = true;
  for(int i=0; i<coverageMatrix.size(); i++)
  {
    if(coverageMatrix[i] == 0)
    {
      res.l = (int)(i/WIN_SIZE);
      res.k = i%((int)(WIN_SIZE));
      res.Fl = res.l;
      res.Fk = res.k;
      finished = false;
      break;
    }
  }
  return res;
}

Mat createChromaticityImage(Mat m)
{
  Mat dest = Mat(im_rows, im_cols, CV_8UC3);
  Vec3b rgb;
  Vec3b RGBsig;
  int teta = 60;
  RGBsig[0] = teta;
  for(int i=0; i< im_rows; i++)
  {
    for(int j=0; j<im_cols; j++)
    {
      rgb = m.at<Vec3b>(i,j);
      RGBsig[1] = 60*log(rgb[0]/(float)(rgb[1]));
      RGBsig[2] = 60*log(rgb[2]/(float)(rgb[1]));
      dest.at<Vec3b>(i,j) = RGBsig;
    }
  }
  return dest;
}

Mat getBSigmaImage(Mat m)
{
  Mat dest = Mat(im_rows, im_cols, CV_8UC3);
  Vec3b rgb;
  Vec3b RGBsig;
  RGBsig[0] = 0;
  for(int i=0; i< im_rows; i++)
  {
    for(int j=0; j<im_cols; j++)
    {
      rgb = m.at<Vec3b>(i,j);
      RGBsig[1] = log(rgb[1]);
      RGBsig[2] = log(rgb[2]/(float)(rgb[1]));
      dest.at<Vec3b>(i,j) = RGBsig;
    }
  }
  return dest;
}


/** *******************************
 *  detectRoad:
 ** *******************************/
vector<double> remove_shadow(Mat image) 
{  
  printf("displaing image\n");
  vector<double> result;
  if(!image.data )
  {    printf( "No image data \n" );     return result; }

  im_rows = image.rows; 
  im_cols = image.cols;
    
  Mat Bsigma = getBSigmaImage(image);
  
  imshow("K", Bsigma);
  
  waitKey(1);

    
  return result;
}



