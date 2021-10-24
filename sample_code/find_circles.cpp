// Headers
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;


// compile command
// cl /EHsc test.cpp /I D:\installs\opencv\opencv\build\include /link /LIBPATH:D:\installs\opencv\opencv\build\x64\vc15\lib opencv_world451.lib



int main(int argc, char* argv[]) {
	
	// Load input video
	//  If your video is in a different source folder than your code, 
	//  make sure you specify the directory correctly!
	VideoCapture input_cap("short_clip_v5.avi");   
	
	// Check validity of target file
	if(!input_cap.isOpened()) {
		std::cout << "Input video not found." << std::endl;
		return -1;
	}
	
	// Set up target output video
	/*	usage: VideoWriter(filename, encoding, framerate, Size)
	 *		in our case, cv_cap_prop_* means "get property of capture"
	 *	 	we want our output to have the same properties as the input!
	 */
	
	VideoWriter output_cap("circled_pieces.avi", 
							VideoWriter::fourcc('H','2','6','4'),
							input_cap.get(CAP_PROP_FPS),
							Size(input_cap.get(CAP_PROP_FRAME_WIDTH),
							input_cap.get(CAP_PROP_FRAME_HEIGHT)));
	
	// Again, check validity of target output file
	if(!output_cap.isOpened()) {
		std::cout << "Could not create output file." << std::endl;
		return -1;
	}
	
	// Loop to read from input one frame at a time, write text on frame, and
	// copy to output video
	Mat frame;
	int count = 0;
	while(input_cap.read(frame)) {
		//cout << "working " << count << "\n";
		
		Mat gray;
		cvtColor(frame, gray, COLOR_BGR2GRAY);
		medianBlur(gray, gray, 5);
		vector<Vec3f> circles;
		HoughCircles(gray, circles, HOUGH_GRADIENT, 1,
			gray.rows/32,
			100, 30, 20, 40
		);
		for(size_t num=0; num < circles.size(); num++){
			Vec3i c = circles[num];
			Point center = Point(c[0], c[1]);
			circle(frame, center, 1, Scalar(0,100,100), 3, LINE_AA);
			int radius = c[2];
			circle(frame, center, radius, Scalar(0,255,0), 3, LINE_AA);
		}
		count++;
		output_cap.write(frame);
	}
	
	
	// free the capture objects from memory
	input_cap.release();
	output_cap.release();
	
	return 1;
	
}