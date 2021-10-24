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

void colorCircle(Mat* img, Mat* dest, Point* center);

int main(int argc, char* argv[]) {
	
	// Load input video
	//  If your video is in a different source folder than your code, 
	//  make sure you specify the directory correctly!
	VideoCapture input_cap("ec_video1.mov");   
	
	// Check validity of target file
	if(!input_cap.isOpened()) {
		std::cout << "Input video not found." << std::endl;
		return -1;
	}

	VideoWriter output_cap("ec_result1.avi", 
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
	const int TAU = 1;
	const int subtractTAU = 255 / TAU;
	// const int TAU_line = 10;
	// const int subtractTAU_line = 255 / TAU_line;
	Point lastCenter = Point(-1, -1);
	uint8_t timeWithoutCircle = 0;
	Mat lastFrame, frame;
	input_cap.read(lastFrame);
	input_cap.read(frame);
	Mat motion_history_image = Mat::zeros(lastFrame.rows, lastFrame.cols, CV_8UC1);
	while (!frame.empty()) {
		// Calculate absolute difference
		Mat diff;
		absdiff(frame, lastFrame, diff);

		// Identify movement (changed pixels)
		cvtColor(diff, diff, COLOR_BGR2GRAY);
		threshold(diff, diff, 10, 255, THRESH_BINARY);

		// Remove noise
		Mat SEdisk = 255*getStructuringElement(MORPH_ELLIPSE, Size(15, 15));
		erode(diff, diff, SEdisk);
		dilate(diff, diff, SEdisk);
		
		// Update MHI (motion_history_image)
		motion_history_image -= subtractTAU;
		bitwise_or(motion_history_image, diff, motion_history_image);

		// Filter MEI
		Mat motion_energy_image;
		threshold(motion_history_image, motion_energy_image, 0, 255, THRESH_BINARY);
		SEdisk = 255*getStructuringElement(MORPH_ELLIPSE, Size(25, 25));
		erode(motion_energy_image, motion_energy_image, SEdisk);
		dilate(motion_energy_image, motion_energy_image, SEdisk);

		Mat frameMovement;
		frame.copyTo(frameMovement);
		for (int row = 0; row < frameMovement.rows; ++row) {
			for (int col = 0; col < frameMovement.cols; ++col) {
				if (motion_energy_image.at<uint8_t>(row, col) <= 0) {
					frameMovement.at<Vec3b>(row, col) = Vec3b(0, 0, 0);
				}
			}
		}

		Mat frameMovement_gry;
		cvtColor(frameMovement, frameMovement_gry, COLOR_BGR2GRAY);

		vector<Vec3f> circle_locs;
		HoughCircles(
			frameMovement_gry, circle_locs,
			HOUGH_GRADIENT, 1,
			100, // minDist between center of circles
			100, 30,
			70, 90 // radius range
		);
		
		// Color circle if moved
		Mat frameCopy;
		frame.copyTo(frameCopy);
		if (circle_locs.size() >= 1) {
			Point curCenter = Point(circle_locs[0][0], circle_locs[0][1]);
			if (lastCenter.x >= 0) {
				// Change circle color
				colorCircle(&frame, &frameCopy, &curCenter);
			}
			lastCenter.x = curCenter.x;
			lastCenter.y = curCenter.y;
			timeWithoutCircle = 0;
		} else {
			if (lastCenter.x >= 0) {
				colorCircle(&frame, &frameCopy, &lastCenter);
			}
			// Forget moving circle after 2 frames of it being stationary
			if (timeWithoutCircle < 3) {
				timeWithoutCircle += 1;
			} else {
				lastCenter.x = -1;
				lastCenter.y = -1;
			}
		}

		// Get new frame
		frame.copyTo(lastFrame);
		input_cap.read(frame);

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

void colorCircle(Mat* img, Mat* dest, Point* center) {
	const int MAX_RADIUS = 78;
	const int MAX_RADIUS_SQ = MAX_RADIUS*MAX_RADIUS;
	Vec3b rgb = img->at<Vec3b>(center->y, center->x);
	int sum = rgb(0) + rgb(1) + rgb(2);
	if (sum > 0 && 255*rgb(2) / sum > 100) {
		for (int row = center->y-MAX_RADIUS; row < center->y+MAX_RADIUS; ++row) {
			for (int col = center->x-MAX_RADIUS; col < center->x+MAX_RADIUS; ++col) {
				int xDiff = center->x - col;
				int yDiff = center->y - row;
				if (xDiff*xDiff + yDiff*yDiff < MAX_RADIUS_SQ) {
					rgb = dest->at<Vec3b>(row, col);
					// sum = rgb(0) + rgb(1) + rgb(2);
					// if (sum > 0 && 255*rgb(2) / sum > 100) {
						dest->at<Vec3b>(row, col) = Vec3b(rgb(2), rgb(1), rgb(0));
					// }
				}
			}
		}
	}
}