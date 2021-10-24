// Headers
#include "find_comp_origin.h"
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// compile command
// cl /EHsc play_vid_test.cpp /I D:\installs\opencv\opencv\build\include /link /LIBPATH:D:\installs\opencv\opencv\build\x64\vc15\lib opencv_world451.lib

Mat RGB2CHR(Mat img) {
	int rows = img.rows;
	int cols = img.cols;
	Mat img_chr = Mat::zeros(rows, cols, CV_32FC2);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			Vec3b rgb = img.at<Vec3b>(row, col);
			float sum_rgb = rgb(0) + rgb(1) + rgb(2);
			if (sum_rgb != 0) {
				img_chr.at<Vec2f>(row, col) = Vec2f(rgb(1) / sum_rgb, rgb(2) / sum_rgb);
			}
		}
	}
	return img_chr;
}
Mat thresholdChromatic(Mat img_chr, float r_min, float r_max, float g_min, float g_max) {
	int rows = img_chr.rows;
	int cols = img_chr.cols;
	Mat thresholded = Mat::zeros(rows, cols, CV_8UC1);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			Vec2f gr = img_chr.at<Vec2f>(row, col);
			if (g_min <= gr(0) && gr(0) <= g_max && r_min <= gr(1) && gr(1) <= r_max) {
				thresholded.at<uint8_t>(row, col) = 255;
			}
		}
	}
	return thresholded;
}
Mat thresholdBlue(Mat img_color, uint8_t min, uint8_t max) {
	int rows = img_color.rows;
	int cols = img_color.cols;
	Mat thresholded = Mat::zeros(rows, cols, CV_8UC1);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			Vec3b rgb = img_color.at<Vec3b>(row, col);
			if (min <= rgb(0) && rgb(0) <= max) {
				thresholded.at<uint8_t>(row, col) = 255;
			}
		}
	}
	return thresholded;
}
Mat thresholdBlueSlide(Mat img_color, uint8_t bRange, uint8_t bStart, uint8_t bEnd, uint8_t rStart, uint8_t rEnd) {
	int rows = img_color.rows;
	int cols = img_color.cols;
	Mat thresholded = Mat::zeros(rows, cols, CV_8UC1);
	float slope = (bStart - bEnd) / (float)(rStart - rEnd);
	float yIntercept = bStart - slope*rStart;
	cout << "Slope: " << slope << "; y-intercept: " << yIntercept << endl;
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			Vec3b rgb = img_color.at<Vec3b>(row, col);
			int rMid = slope*rgb(2) + yIntercept;
			if (rMid - bRange <= rgb(0) && rgb(0) <= rMid + bRange) {
				thresholded.at<uint8_t>(row, col) = 255;
			}
		}
	}
	return thresholded;
}

int main(int argc, char* argv[]) {
	
	// Load input video
	//  If your video is in a different source folder than your code, 
	//  make sure you specify the directory correctly!
	// VideoCapture input_cap("p3_video1.avi");   
	
	// Check validity of target file
	// if(!input_cap.isOpened()) {
	// 	std::cout << "Input video not found." << std::endl;
	// 	return -1;
	// }
	
	namedWindow("output", WINDOW_AUTOSIZE);

	Mat rollDice1Pixel = imread("RollDice1Pixel.png", IMREAD_COLOR);
	cout << rollDice1Pixel << endl;
	Mat RollDice1PixelDark = imread("RollDice1PixelDark.png", IMREAD_COLOR);
	cout << RollDice1PixelDark << endl;
	Mat rollDice2Pixel = imread("RollDice2Pixel.png", IMREAD_COLOR);
	cout << rollDice2Pixel << endl;
	Mat RollDice2PixelDark = imread("RollDice2PixelDark.png", IMREAD_COLOR);
	cout << RollDice2PixelDark << endl;
	
	Mat SERollDice = imread("RollDiceStruct.png", IMREAD_COLOR);
	cvtColor(SERollDice, SERollDice, COLOR_RGB2GRAY);
	// threshold(SERollDice, SERollDice, 200, 255, THRESH_BINARY);
	// erode(SERollDice, SERollDice, 255*Mat::ones(5, 5, CV_8UC1));
	threshold(SERollDice, SERollDice, 10, 255, THRESH_BINARY);
	imshow("SERollDice", SERollDice);

	// Loop to read from input one frame at a time
	Mat frame;
	int count = 0;
	const int numSearches = 2;
	const char* searchFiles[numSearches] = {
		"RollDice1.png",
		"RollDice2.png"
	};
	for (int searchInd = 0; searchInd < numSearches; ++searchInd) {

		Mat img_color, img_gry, img_bw;
		img_color = cv::imread(searchFiles[searchInd], IMREAD_COLOR);

		// IDEA: look for colors which make of the roll dice display.
		// If a threadhold is reached, then we can say it's good

		// OR we structing element the "Roll Dice", then
		
		Mat gray;
		cvtColor(img_color, gray, COLOR_BGR2GRAY);
		medianBlur(gray, gray, 5);
		vector<Vec3f> circles;
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
		gray.rows/32,
		100, 30, 20, 40
		);
		for(size_t num=0; num < circles.size(); num++){
			Vec3i c = circles[num];
			Point center = Point(c[0], c[1]);
			circle(img_color, center, 1, Scalar(0,100,100), 3, LINE_AA);
			int radius = c[2];
			circle(img_color, center, radius, Scalar(0,255,0), 3, LINE_AA);
		}
		
		imshow("output", img_color);
		Mat img_color_threshBlue = thresholdBlue(img_color, 50, 255);
		Mat rollDiceLocMat;
		erode(img_color_threshBlue, rollDiceLocMat, SERollDice);

		Point2i rollDiceOrigin = find_comp_origin(rollDiceLocMat);
		for (int row = rollDiceOrigin.y - (int)(SERollDice.rows*0.6); row < rollDiceOrigin.y + SERollDice.rows*0.5; ++row) {
			for (int col = rollDiceOrigin.x - (int)(SERollDice.cols*0.6); col < rollDiceOrigin.x + SERollDice.cols*0.5; ++col) {
				Vec3b rgb = img_color.at<Vec3b>(row, col);
				if (rgb(2) > 160 && rgb(1) > 160) { // if contains red (looking for white)
					img_color.at<Vec3b>(row, col) = Vec3b(0, 0, rgb(2));
				}
			}
		}
		imshow("COLORED", img_color);
		waitKey(0);
		

		// wait for ESC key to be pressed
		// if(waitKey(30) == 27)
		// {
		// 	imwrite("RollDiceChr.png", img_chr);
		// 	waitKey(0);
		// }
	}
	
	// free the capture objects from memory
	// input_cap.release();
	
	return 1;
	
}