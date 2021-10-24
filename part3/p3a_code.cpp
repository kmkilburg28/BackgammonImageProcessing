// Headers
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

	VideoWriter output_cap("p3a_result1.avi", 
	                        VideoWriter::fourcc('H','2','6','4'),
	                        input_cap.get(CAP_PROP_FPS),
	                        Size(input_cap.get(CAP_PROP_FRAME_WIDTH),
	                        input_cap.get(CAP_PROP_FRAME_HEIGHT)));
	
	// Again, check validity of target output file
	if(!output_cap.isOpened()) {
		std::cout << "Could not create output file." << std::endl;
		return -1;
	}
	
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

		Mat frameCopy;
		frame.copyTo(frameCopy);
		vector<Vec3f> circle_locs;
		const int MAX_RADIUS = 45;
		const int EPS_RADIUS = MAX_RADIUS + 0;
		HoughCircles(
			img_gry, circle_locs,
			HOUGH_GRADIENT, 1,
			img_gry.rows / 32, // minDist between center of circles
			100, 30,
			35, MAX_RADIUS // radius range
		);
		for (int i = 0; i < circle_locs.size(); i++) {
			Point center = Point(circle_locs[i][0], circle_locs[i][1]);
			Vec3b rgb = frame.at<Vec3b>(center.y, center.x);
			int sum = rgb(0) + rgb(1) + rgb(2);
			if (sum > 0 && 255*rgb(2) / sum > 100) {
				for (int row = center.y-EPS_RADIUS; row < center.y+EPS_RADIUS; ++row) {
					for (int col = center.x-EPS_RADIUS; col < center.x+EPS_RADIUS; ++col) {
						int xDiff = center.x - col;
						int yDiff = center.y - row;
						if (xDiff*xDiff + yDiff*yDiff < EPS_RADIUS*EPS_RADIUS) {
							rgb = frame.at<Vec3b>(row, col);
							sum = rgb(0) + rgb(1) + rgb(2);
							if (sum > 0 && (255*rgb(2) / sum > 105 && 255*rgb(1) / sum < 70) && 
								(!(170 < 255*rgb(2) / sum &&
								   40 < 255*rgb(1) / sum && sum > 60) &&
								 !(210 < 255*rgb(2) / sum &&
								   20 < 255*rgb(1) / sum && sum > 20))) {
							// if (sum > 0 && ((255*rgb(2) / sum > 200 && rgb(2) > 110) || (255*rgb(2) / sum > 140 && rgb(2) > 0))) {
							// if (sum > 0 && 255*rgb(2) / sum > 110 && rgb(2) > 95 && rgb(1) < 100 && rgb(0) < 100) {
							// if (sum > 0 && 255*rgb(2) / sum > 110 && rgb(2) > 95) {
								frameCopy.at<Vec3b>(row, col) = Vec3b(rgb(2), rgb(1), rgb(0));
							}
						}
					}
				}
			}
		}
		
		// Mat frameRed = Mat::zeros(frame.rows, frameCopy.cols, CV_8UC1);
		// for (int row = 0; row < frameRed.rows; ++row) {
		// 	for (int col = 0; col < frameRed.cols; ++col) {
		// 		Vec3b rgb = frame.at<Vec3b>(row, col);
		// 		int sum = rgb(0) + rgb(1) + rgb(2);
		// 		if (sum > 0)
		// 			frameRed.at<uint8_t>(row, col) = 255*rgb[2] / sum;
		// 	}
		// }
		// // frameCopyBlue
		// imshow("frameRed", frameRed);
		// Mat SEdisk = 255*getStructuringElement(MORPH_ELLIPSE, Size(17, 17));
		// threshold(frameRed, frameRed, 250, 255, THRESH_BINARY);
		// dilate(frameRed, frameRed, SEdisk);
		// erode(frameRed, frameRed, SEdisk);
		// imshow("frameRedNew", frameRed);
		// // for (int row = 0; row < frameCopy.rows; ++row) {
		// // 	for (int col = 0; col < frameCopy.cols; ++col) {
		// // 		if (frameRed.at<uint8_t>(row, col) > 0) {
		// // 		}
		// // 	}
		// // }

		
		int rows = frame.rows;
		int cols = frame.cols;
		for (int row = 0; row < rows; ++row) {
			for (int col = 0; col < cols; ++col) {
				Vec3b rgb = frameCopy.at<Vec3b>(row, col);
				int sum = rgb(0) + rgb(1) + rgb(2);
				if (sum > 0 && 255*rgb(2) / sum > 155 && rgb(2) > 110) {
					frameCopy.at<Vec3b>(row, col) = Vec3b(rgb(2), rgb(1), rgb(0));
		
					// // Smooth neighbors (reds that were too dark)
					// uint8_t neighbor_size = 2;
					// for (int row_neighbor = row-neighbor_size; row_neighbor < row+neighbor_size; ++row_neighbor) {
					// 	for (int col_neighbor = col-neighbor_size; col_neighbor < col+neighbor_size; ++col_neighbor) {
					// 		rgb = frame.at<Vec3b>(row_neighbor, col_neighbor);
					// 		sum = rgb(0) + rgb(1) + rgb(2);
					// 		if (sum > 0 && 255*rgb(2) / sum > 100) {
					// 			frameCopy.at<Vec3b>(row_neighbor, col_neighbor) = Vec3b(rgb(2), rgb(1), rgb(0));
					// 		}
					// 	}
					// }
				}
			}
		}

		// wait for ESC key to be pressed
		if(waitKey(30) == 27)
		{
			break;
		}
		output_cap.write(frameCopy);
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