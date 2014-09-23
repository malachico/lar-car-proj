#ifndef SIMPLEGUI__H
#define SIMPLEGUI__H

#include <vector>
#include <string>
#include <opencv2/opencv.hpp>
#include <map>

namespace SimpleGUI
{
	extern class GUIObject* __hoverObj;
	
	class GUIObject
	{
		friend class GUIWindow;
		public:
			bool isVisible();
			void setVisible(bool);
			
			std::pair<int,int> getPos();
			std::pair<int,int> getDims();
			void setPos(std::pair<int, int>);
			void setDims(std::pair<int, int>);
			virtual void draw(cv::Mat& img) = 0;	
			
			virtual bool checkBounds(int x, int y);
			virtual void onMouse(int event, int x, int y, int flags);
			
		protected:
			GUIObject(int x, int y, int width, int height);	
			//(0,0) is top left corner of parent canvas
			int posX, posY, width, height;
			bool visible;
	};	
	
	class Canvas : public GUIObject
	{
		public:
			Canvas(int width, int height, bool transparent = true);
			Canvas(int x, int y, int width, int height, bool transparent = true);
			virtual ~Canvas();
			void addObject(GUIObject* obj);
			void removeObject(GUIObject* obj);
			virtual void draw(cv::Mat& img);
			virtual void onMouse(int event, int x, int y, int flags);
			virtual bool checkBounds(int x, int y);
			void setForegroundColor(cv::Scalar);
			
		private:
			std::vector<GUIObject*> _list;	
			bool 					_transparent;
			cv::Scalar 				_foregroundColor;
	};
	
	class Image : public GUIObject
	{
		public:
			Image(cv::Mat m); //0 0 natural width and height
			Image(cv::Mat m, int x, int y); //natural width and height
			Image(cv::Mat m, int x, int y, int width, int height);
			void draw(cv::Mat& img);
			void setImage(cv::Mat img);
		private:
			cv::Mat _img;
	};
	
	class ImageButton : public GUIObject
	{
		public:
			ImageButton(int x, int y, int width, int height, cv::Mat pressed, cv::Mat released);
			ImageButton(std::string title, int x, int y, int width, int height, cv::Mat pressed, cv::Mat released);
			~ImageButton();
			void onMouse(int event, int x, int y, int flags);
			void draw(cv::Mat& img);
			void setHoverImg(cv::Mat& hover);
			bool checkBounds(int x, int y);
			void setString(std::string str);
			std::string getString();
			void setListener(void (*listener)(ImageButton* button, int rel_x, int rel_y));
			
		protected:
			void 			_reposition();
			cv::Mat 		_pressed, _released, _hover;
			class CString* 	_title;
			bool 			_isPressed;
			void 			(*_listener)(ImageButton* button, int rel_x, int rel_y);
	};
	
	class SimpleButton : public ImageButton
	{
		public:
			SimpleButton(int x, int y, int width, int height);
			SimpleButton(std::string title, int x, int y, int width, int height);
			void draw(cv::Mat& img);
			
			void setColors(cv::Scalar pressed, cv::Scalar released, cv::Scalar fontColor);
	};
	
	class CString : public GUIObject
	{
		public:
			CString(std::string str, int size);
			CString(std::string str, int size, int x, int y);
			void draw(cv::Mat& img);
			void setColor(cv::Scalar color);
			void setString(std::string str);
			std::string getString();
			
		private:
			std::string 	_str;
			int 			_size;
			cv::Scalar 		_color;
	};
	
	class GUIWindow
	{
		public:
			GUIWindow(Canvas* canvas, std::string name = "SimpleGUI");
			~GUIWindow();
			void draw();
			void update();
			
			static void MouseCallback(int event, int x, int y, int flags, void* ptr);
		private:
			Canvas* 		_canvas;
			cv::Mat 		_window;
			std::string 	_name;
	};
	
}



#endif
