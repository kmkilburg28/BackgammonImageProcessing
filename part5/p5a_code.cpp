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

int main(int argc, char* argv[]) {
	
	// Load input video
	//  If your video is in a different source folder than your code, 
	//  make sure you specify the directory correctly!
	VideoCapture input_cap("p5_video1.avi");   
	
	// Check validity of target file
	if(!input_cap.isOpened()) {
		std::cout << "Input video not found." << std::endl;
		return -1;
	}

	VideoWriter output_cap("p5a_result1.avi", 
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
	const int TAU = 26;
	const int subtractTAU = 255 / TAU;
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
		threshold(diff, diff, 15, 255, THRESH_BINARY);

		// // Remove noise
		// Mat SEdisk = 255*getStructuringElement(MORPH_ELLIPSE, Size(5, 5));
		// erode(diff, diff, SEdisk);
		// dilate(diff, diff, SEdisk);
		
		// Update MHI (motion_history_image)
		motion_history_image -= subtractTAU;
		bitwise_or(motion_history_image, diff, motion_history_image);

		// Get new frame
		frame.copyTo(lastFrame);
		input_cap.read(frame);

		// wait for ESC key to be pressed
		if(waitKey(30) == 27)
		{
			break;
		}

		Mat output_MHI;
		cvtColor(motion_history_image, output_MHI, COLOR_GRAY2BGR);
		output_cap.write(output_MHI);
	}
	
	// free the capture objects from memory
	input_cap.release();
	output_cap.release();
	
	return 1;
}