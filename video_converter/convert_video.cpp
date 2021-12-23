// Headers
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

// compile command
// cl /EHsc play_vid_test.cpp /I D:\installs\opencv\opencv\build\include /link /LIBPATH:D:\installs\opencv\opencv\build\x64\vc15\lib opencv_world451.lib


int main(int argc, char* argv[]) {
	
	if (argc <= 3) {
		std::cout << "Input video path not provided." << std::endl;
		return -1;
	}
	// Load input video
	//  If your video is in a different source folder than your code, 
	//  make sure you specify the directory correctly!
	VideoCapture input_cap((string(argv[3]) + string(".") + string(argv[1])).c_str());   
	
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
	
	VideoWriter output_cap((string(argv[3]) + string(".") + string(argv[2])).c_str(), 
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
	while(input_cap.read(frame)) {
		output_cap.write(frame);
	}
	
	
	// free the capture objects from memory
	input_cap.release();
	output_cap.release();
	
	return 1;
	
}