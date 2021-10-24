// Headers
#include <find_comp_origin.h>
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// compile command
// cl /EHsc play_vid_test.cpp /I D:\installs\opencv\opencv\build\include /link /LIBPATH:D:\installs\opencv\opencv\build\x64\vc15\lib opencv_world451.lib

Mat thresholdBlue(Mat img_color, uint8_t min, uint8_t max);

int main(int argc, char* argv[]) {
	
	// Load input video
	//  If your video is in a different source folder than your code, 
	//  make sure you specify the directory correctly!
	VideoCapture input_cap("p3_video1.avi");   
	
	// Check validity of target file
	if(!input_cap.isOpened()) {
		std::cout << "Input video not found." << std::endl;
		return -1;
	}

	VideoWriter output_cap("p3b_result1.avi", 
	                        VideoWriter::fourcc('H','2','6','4'),
	                        input_cap.get(CAP_PROP_FPS),
	                        Size(input_cap.get(CAP_PROP_FRAME_WIDTH),
	                        input_cap.get(CAP_PROP_FRAME_HEIGHT)));
	
	// Again, check validity of target output file
	if(!output_cap.isOpened()) {
		std::cout << "Could not create output file." << std::endl;
		return -1;
	}

	Mat SERollDice = imread("RollDiceStruct.png", IMREAD_COLOR);
	cvtColor(SERollDice, SERollDice, COLOR_RGB2GRAY);
	threshold(SERollDice, SERollDice, 10, 255, THRESH_BINARY);
	
	namedWindow("output", WINDOW_AUTOSIZE);
	
	// Loop to read from input one frame at a time
	Mat frame;
	while(input_cap.read(frame)) {

		// IDEA: look for colors which make of the roll dice display.
		// If a threadhold is reached, then we can say it's good

		// OR we structing element the "Roll Dice", then
		
		Mat img_gry;
		cvtColor(frame, img_gry, COLOR_BGR2GRAY);
		medianBlur(img_gry, img_gry, 5);
		
		Mat img_color_threshBlue = thresholdBlue(frame, 60, 255);
		dilate(img_color_threshBlue, img_color_threshBlue, 255*Mat::ones(5, 5, CV_8UC1));
		Mat rollDiceLocMat;
		erode(img_color_threshBlue, rollDiceLocMat, SERollDice);

		Point2i rollDiceOrigin = find_comp_origin(rollDiceLocMat);
		if (rollDiceOrigin.x >= 0 && rollDiceOrigin.y >= 0) {
			for (int row = rollDiceOrigin.y - (int)(SERollDice.rows*0.6); row < rollDiceOrigin.y + SERollDice.rows*0.5; ++row) {
				for (int col = rollDiceOrigin.x - (int)(SERollDice.cols*0.6); col < rollDiceOrigin.x + SERollDice.cols*0.5; ++col) {
					Vec3b rgb = frame.at<Vec3b>(row, col);
					if (rgb(2) > 160 && rgb(1) > 160) { // if contains red (looking for white)
						frame.at<Vec3b>(row, col) = Vec3b(0, 0, rgb(2));
					}
				}
			}
		}

		// wait for ESC key to be pressed
		if(waitKey(30) == 27)
		{
			break;
		}
		output_cap.write(frame);
	}
	
	// free the capture objects from memory
	input_cap.release();
	output_cap.release();
	
	return 1;
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