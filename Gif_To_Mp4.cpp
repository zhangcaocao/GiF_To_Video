// Gif_To_Mp4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "opencv2/opencv.hpp"

using namespace cv;
using namespace std;


Mat Gif_To_Mat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT fif);
void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message);
bool Gif_Load(const string &filename);
bool Jpg_To_Video();
bool Show_Video();

unsigned char bpp;
BYTE *bits;
bool Load_flag;
int width;
int height;
FREE_IMAGE_FORMAT fif = FIF_UNKNOWN;
Mat Src_Gif;


int _tmain(int argc, _TCHAR* argv[])
{
	FreeImage_Initialise();
	Load_flag = Gif_Load("Gif_2.gif");

	if (Load_flag == TRUE)
	{
		Jpg_To_Video();
		Show_Video();
		cout << "load file successful!" << endl;
	}
	return 0;
}

//-------------  GIF文件的载入
//

bool Gif_Load(const string &filename)
{
	FIBITMAP *dib = 0;
	FIMULTIBITMAP *bitmap = 0;
	FIBITMAP * pFrame;

	fif = FreeImage_GetFileType(filename.c_str(), 0);
	if (fif == FIF_UNKNOWN)						fif = FreeImage_GetFIFFromFilename(filename.c_str());
	if (fif == FIF_UNKNOWN)						return false;
	if (FreeImage_FIFSupportsReading(fif))		dib = FreeImage_Load(fif, filename.c_str());
	if (!dib)									return false;//dib Load failed	

	//bpp = FreeImage_GetBPP(dib);
	bits = (BYTE*)FreeImage_GetBits(dib);
	width = FreeImage_GetWidth(dib);
	height = FreeImage_GetHeight(dib);

	cout << "Load The File:   " << filename.c_str()  << endl;
	cout << "The File's width: " << width << endl;
	cout << "The File's height: " << height << endl;

	if ((bits == 0) || (width == 0) || (height == 0))		return false;

	bitmap = FreeImage_OpenMultiBitmap(fif, filename.c_str(), 0, 0, 1, GIF_DEFAULT);
	if (bitmap == NULL)
	{
		cout << "BitMap == Null" << endl;
		return FALSE;
	}
	
	int count = FreeImage_GetPageCount(bitmap);//获取帧数；

	for (int i = 0; i <=count; i++)
	{
		pFrame = FreeImage_LockPage(bitmap, i);
		//cout << "pFrame:" << pFrame << endl;
		Src_Gif = Gif_To_Mat(pFrame, fif);
		
		string Src_Gif_Name = to_string(i);
		imwrite(Src_Gif_Name + ".jpg", Src_Gif);
		FreeImage_UnlockPage(bitmap, pFrame, 1);
	}

	FreeImage_Unload(dib);
	FreeImage_DeInitialise();
	Load_flag = TRUE;
	return Load_flag;
}

/*
将当前文件夹中的 “.jpg” 生成为视频文件；

*/

bool Jpg_To_Video()
{

	VideoWriter video("output.avi", CV_FOURCC('M', 'P', '4', '2'), 25.0, Size(150, 131));
	String File_Name = "*.jpg";
	vector <String> fn;
	glob(File_Name, fn, false);//遍历文件夹的图片/文件
	size_t size = fn.size();
	cout << "Jpg_To_Video size:" << size << endl;
	cout << "开始将图片文件写入视频" << endl;
	for (size_t i = 0; i < size; i++)
	{
		Mat image = imread(fn[i]);
		//imshow(to_string(i), image);
		//resize(image, image, Size(640, 480));
		video.write(image);
	}

	cout << "写入 成功!" << endl;

	return TRUE;
}

/*

显示生成的视频文件；

*/

bool Show_Video()
{
	cout << "Show The Video.." << endl;

	VideoCapture video_capture("output.avi");
	if (!video_capture.isOpened())		return FALSE;


	double totalFrameNumber = video_capture.get(CV_CAP_PROP_FRAME_COUNT);
	cout << "整个视频共" << totalFrameNumber << "帧" << endl;
	//设置开始帧()
	long frameToStart = 1;
	video_capture.set(CV_CAP_PROP_POS_FRAMES, frameToStart);
	cout << "从第" << frameToStart << "帧开始读" << endl;

	//设置结束帧
	int frameToStop = totalFrameNumber;

	if (frameToStop < frameToStart)
	{
		cout << "结束帧小于开始帧，程序错误，即将退出！" << endl;
		return FALSE;
	}
	else
	{
		cout << "结束帧为：第" << frameToStop << "帧" << endl;
	}

	//获取帧率
	double rate = video_capture.get(CV_CAP_PROP_FPS);
	cout << "帧率为:" << rate << endl;

	//定义一个用来控制读取视频循环结束的变量
	bool stop = false;
	//承载每一帧的图像
	Mat frame;
	//显示每一帧的窗口
	namedWindow("Extracted frame");
	//两帧间的间隔时间:
	double delay = rate;

	//利用while循环读取帧
	//currentFrame是在循环体中控制读取到指定的帧后循环结束的变量
	long currentFrame = frameToStart;


	while (!stop)
	{
		//读取下一帧
		if (!video_capture.read(frame))
		{
			cout << "读取视频结束" << endl;
			return FALSE;
		}

		imshow("Extracted frame", frame);

		cout << "正在读取第" << currentFrame << "帧" << endl;

		//waitKey(int delay=0)当delay ≤ 0时会永远等待；当delay>0时会等待delay毫秒
		//当时间结束前没有按键按下时，返回值为-1；否则返回按键
		int c = waitKey(delay);

		//按下ESC键退出视频的帧流显示
		if ((char)c == 27 || currentFrame > frameToStop)
		{
			stop = true;
		}
		//按下按键后会停留在当前帧，等待下一次按键
		if (c >= 0)
		{
			waitKey(0);
		}
		currentFrame++;
	}
	//关闭视频文件
	video_capture.release();
	waitKey(0);
	return TRUE;
}
//-------------  Conversion Gif to Mat
//

Mat Gif_To_Mat(FIBITMAP* fiBmp, const FREE_IMAGE_FORMAT fif)
{
	if (fiBmp == NULL || fif != FIF_GIF)
	{
		return Mat();
	}

	//int width = FreeImage_GetWidth(fiBmp);
	//int height = FreeImage_GetHeight(fiBmp);
	
	BYTE intensity;
	BYTE* PIintensity = &intensity;
	if (FreeImage_GetBPP(fiBmp) != 8)
		fiBmp = FreeImage_ConvertTo8Bits(fiBmp);

	RGBQUAD* pixels = new RGBQUAD;
	pixels = FreeImage_GetPalette(fiBmp);

	Mat img = Mat::zeros(height, width, CV_8UC3);

	uchar *p;

	for (int i = 0; i < height; i++)
	{
		p = img.ptr<uchar>(i);
		for (int j = 0; j < width; j++)
		{
			FreeImage_GetPixelIndex(fiBmp, j, height - i, PIintensity);
			p[3 * j] = pixels[intensity].rgbBlue;
			p[3 * j + 1] = pixels[intensity].rgbGreen;
			p[3 * j + 2] = pixels[intensity].rgbRed;
		}
	}

	return img;
}


//--------------  ErrorHandler  --------------
// FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
// In your main program …
//FreeImage_SetOutputMessage(FreeImageErrorHandler);


void FreeImageErrorHandler(FREE_IMAGE_FORMAT fif, const char *message)
{
	cout << "\n***" << endl;
	if (fif != FIF_UNKNOWN)
	{
		cout  << FreeImage_GetFormatFromFIF(fif) << "\n" << endl;

	}
	cout << message << endl;
	cout << "***\n" << endl;
}

