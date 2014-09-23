#include "SimpleGUI.h"

using namespace cv;
using namespace std;


namespace SimpleGUI
{
	GUIObject* __hoverObj = NULL;
	//-------------------GUIWindow--------------------//
	
	GUIWindow::GUIWindow(Canvas* canvas, string name) : _canvas(canvas)
	{
		_window = Mat(canvas->getDims().second, canvas->getDims().first, CV_8UC3);
		_name = name;
		namedWindow(name);
		setMouseCallback(name, &MouseCallback, (void*)this);
	}
	
	GUIWindow::~GUIWindow()
	{
		delete _canvas;
	}
	
	void GUIWindow::update()
	{
		_canvas->draw(_window);
	}
	
	void GUIWindow::draw()
	{
		imshow(_name, _window);
		waitKey(1);
	}
	
	void GUIWindow::MouseCallback(int event, int x, int y, int flags, void* ptr)
	{
		GUIWindow* wnd = (GUIWindow*)ptr;
		wnd->_canvas->onMouse(event, x, y, flags);
		if(__hoverObj && !__hoverObj->checkBounds(x,y)) __hoverObj = NULL;
		if(event != EVENT_MOUSEMOVE) 
		{
			wnd->update();
			wnd->draw();	
		}
		else
		{
			
		}
	}
	
	//-------------------GUIObject--------------------//
	
	GUIObject::GUIObject(int x, int y, int width, int height)
	{
		this->posX = x;
		this->posY = y;
		this->width = width;
		this->height = height;
		this->visible = true;
	}
	
 	void GUIObject::setVisible(bool flag)
	{
		visible = flag;
	}
	
	bool GUIObject::isVisible()
	{
		return visible;
	}
	
	pair<int, int> GUIObject::getPos()
	{
		return pair<int, int>(posX, posY);
	}	
	
	pair<int, int> GUIObject::getDims()
	{
		return pair<int, int>(width, height);
	}	
	
	bool GUIObject::checkBounds(int x, int y)
	{
		return false;
	}
	
	void GUIObject::onMouse(int event, int x, int y, int flags)
	{
		
	}
	
	void GUIObject::setPos(pair<int, int> pos)
	{
		posX = pos.first;
		posY = pos.second;
	}
	
	void GUIObject::setDims(pair<int, int> dim)
	{
		width = dim.first;
		height = dim.second;
	}
	
	//-------------------Canvas-----------------------//
	
	Canvas::Canvas(int width, int height, bool transparent) : GUIObject(0,0,width,height)
	{
		_transparent = transparent;
		_foregroundColor = Scalar(0,0,0);
	}
	
	Canvas::Canvas(int x, int y, int width, int height, bool transparent) : GUIObject(x,y,width,height)
	{
		_transparent = transparent;
		_foregroundColor = Scalar(0,0,0);
	}
	
	Canvas::~Canvas()
	{
		for(vector<GUIObject*>::iterator it = _list.begin(); it != _list.end(); it++)
		{
			GUIObject* obj = *it;
			delete obj;
		}
		_list.clear();
	}
	
	void Canvas::setForegroundColor(Scalar color)
	{
		this->_foregroundColor = color;
	}
	
	void Canvas::draw(Mat& img)
	{
		int x = min(posX, img.cols-1), y = min(posY, img.rows-1);
		int w = min(width, img.cols-x), h = min(height, img.rows-y);
		Rect roi(Point(x,y), Size(w,h));
		Mat destinationROI = img(roi);
		Mat myImg = Mat(height, width, CV_8UC3, _foregroundColor);
		if(_transparent) destinationROI.copyTo(myImg);
		for(vector<GUIObject*>::iterator it = _list.begin(); it != _list.end(); it++)
		{
			GUIObject* obj = *it;
			obj->draw(myImg);
		}
		
		myImg = myImg(Rect(Point(0,0),Size(w,h)));
		myImg.copyTo(destinationROI);
		//Mat myImg = _img(Rect(0,0,w,h));
		
	}
	
	void Canvas::addObject(GUIObject* obj)
	{
		_list.push_back(obj);
	}
	
	void Canvas::removeObject(GUIObject* obj)
	{
		for(vector<GUIObject*>::iterator it = _list.begin(); it != _list.end(); it++)
		{
			if(obj == *it) 
			{
				_list.erase(it);
			}
		}
	}
	
	bool Canvas::checkBounds(int x, int y)
	{
		return x >= posX && x < posX+width && y >= posY && y < posY+height;
	}
	
	void Canvas::onMouse(int event, int x, int y, int flags)
	{
		for(vector<GUIObject*>::iterator it = _list.begin(); it != _list.end(); it++)
		{
			GUIObject* obj = *it;
			if(obj->checkBounds(x-posX, y-posY))
			{
				obj->onMouse(event, x, y, flags);
				return;
			}
		}
		
	}
	
	//-------------------Image------------------------//
	
	Image::Image(Mat m, int x, int y) : GUIObject(x,y,m.cols,m.rows)
	{
		this->_img = m;
	}
	
	Image::Image(Mat m, int x, int y, int width, int height) : GUIObject(x,y,width,height) 
	{
		resize(m, _img, Size(width, height));
	}
	
	Image::Image(Mat m) : GUIObject(0,0,m.cols,m.rows)
	{
		this->_img = m;
	}
	
	void Image::setImage(Mat img)
	{
		resize(img, _img, Size(width, height));
	}
	
	void Image::draw(Mat& img)
	{
		int x = min(posX, img.cols-1), y = min(posY, img.rows-1);
		int w = min(width, img.cols-x), h = min(height, img.rows-y);
		Rect roi(Point(x,y), Size(w,h));
		Mat destinationROI = img(roi);
		Mat myImg = _img(Rect(0,0,w,h));
		myImg.copyTo(destinationROI);
	}
	
	//-------------------String-----------------------//
	
	CString::CString(string str, int size) : GUIObject(0,0,str.length()*size,size)
	{
		_str = str; 
		_size = size;
		_color = Scalar(0, 255, 0);
	}	
	
	CString::CString(string str, int size, int x, int y) : GUIObject(x,y,str.length()*size,size)
	{
		_str = str;
		_size = size;
		_color = Scalar(0, 255, 0);
	}
	
	void CString::draw(Mat& img)
	{
		//addText(img, _str, Point(posX,posY+_size), fontQt("Arial"));
		putText(img, _str, Point(posX,posY+_size), FONT_HERSHEY_COMPLEX_SMALL, 0.07*_size, _color);
	}
	
	void CString::setColor(Scalar color)
	{
		_color = color;
	}
	
	string CString::getString()
	{
		return _str;
	}
	
	void CString::setString(string str)
	{
		_str = str;
	}
	
	//-----------------ImageButton--------------------//
	
	ImageButton::ImageButton(int x, int y, int width, int height, Mat pressed, Mat released) 
		: GUIObject(x,y,width,height)
	{
		_title = NULL;
		resize(pressed, _pressed, Size(width, height));
		resize(released, _released, Size(width, height));
		_isPressed = false;
		_hover = released;
		_listener = NULL;
	}
	
	ImageButton::ImageButton(string title, int x, int y, int width, int height, Mat pressed, Mat released) 
		: GUIObject(x,y,width,height)
	{
		resize(pressed, _pressed, Size(width, height));
		resize(released, _released, Size(width, height));
		int fontsize = width/(title.length()+4);
		_title = new CString(title, fontsize, (2*x+width)/2-title.length()*fontsize/2, y+height/2-fontsize/2);
		_isPressed = false;
		_hover = released;
		_listener = NULL;
	}
	
	void ImageButton::setListener(void (*listener)(ImageButton* button, int rel_x, int rel_y))
	{
		_listener = listener;
	}
	
	void ImageButton::setString(std::string str)
	{
		if(_title)
		{
			_title->setString(str);
			_reposition();	
		}
		else
		{
			int fontsize = width/(_title->getString().length()+4);
			_title = new CString(str, fontsize, (2*posX+width)/2-str.length()*fontsize/2, posY+height/2-fontsize/2);
		}
	}
	
	void ImageButton::_reposition()
	{
		int fontsize = width/(_title->getString().length()+4);
		_title->setPos(pair<int, int>((2*posX+width)/2-_title->getString().length()*fontsize/2, posY+height/2-fontsize/2));
	}
	
	ImageButton::~ImageButton()
	{
		if(_title) delete _title;
	}
	
	string ImageButton::getString()
	{
	  return _title->getString();
	}

	void ImageButton::draw(Mat& img)
	{
		int x = min(posX, img.cols-1), y = min(posY, img.rows-1);
		int w = min(width, img.cols-x), h = min(height, img.rows-y);
		Rect roi(Point(x,y), Size(w,h));
		Mat destinationROI = img(roi);
		Mat myImg = _isPressed ? _pressed(Rect(0,0,w,h)) : ((__hoverObj == this)  ? _hover(Rect(0,0,w,h)) : _released(Rect(0,0,w,h)));
		myImg.copyTo(destinationROI);
		if(_title) _title->draw(img);
	}	
	
	void ImageButton::setHoverImg(cv::Mat& hover)
	{
		resize(hover, _hover, Size(width, height));
	}
	
	bool ImageButton::checkBounds(int x, int y)
	{
		return x >= posX && x < posX+width && y >= posY && y < posY+height;
	}
	
	void ImageButton::onMouse(int event, int x, int y, int flags)
	{
		__hoverObj = this;
		if  (event == EVENT_LBUTTONUP)
		{
			_isPressed = false;
			if(_listener != NULL) _listener(this, x-posX, y-posY);
		}
		else if  (event == EVENT_LBUTTONDOWN)
		{
			_isPressed = true;
		}
		else if (event == EVENT_MOUSEMOVE) 
		{
			
		}	
	}
	
	//-----------------SimpleButton-------------------//
	
	SimpleButton::SimpleButton(int x, int y, int width, int height)
		: ImageButton(x,y,width,height,Mat(height, width, CV_8UC3, Scalar(100, 100, 100)), Mat(height, width, CV_8UC3, Scalar(100, 100, 100)))
	{
		
	}
	
	SimpleButton::SimpleButton(string title, int x, int y, int width, int height)
		: ImageButton(title, x,y,width,height,Mat(height, width, CV_8UC3, Scalar(70, 70, 70)), Mat(height, width, CV_8UC3, Scalar(130, 130, 130)))
	{
		_hover = Mat(height, width, CV_8UC3, Scalar(150, 150, 150));
		copyMakeBorder(_pressed, _pressed, 1,1,1,1, BORDER_CONSTANT);
		resize(_pressed, _pressed, Size(width, height));
		copyMakeBorder(_released, _released, 1,1,1,1, BORDER_CONSTANT);
		resize(_released, _released, Size(width, height));
		copyMakeBorder(_hover, _hover, 1,1,1,1, BORDER_CONSTANT);
		resize(_hover, _hover, Size(width, height));
	}
	
	void SimpleButton::draw(cv::Mat& img)
	{
		ImageButton::draw(img);
	}
	
	void SimpleButton::setColors(Scalar pressed, Scalar released, Scalar font)
	{
		_pressed = Mat(height, width, CV_8UC3, pressed);
		_released = Mat(height, width, CV_8UC3, released);
		_title->setColor(font);
		copyMakeBorder(_pressed, _pressed, 1,1,1,1, BORDER_CONSTANT);
		resize(_pressed, _pressed, Size(width, height));
		copyMakeBorder(_released, _released, 1,1,1,1, BORDER_CONSTANT);
		resize(_released, _released, Size(width, height));
	}
}
