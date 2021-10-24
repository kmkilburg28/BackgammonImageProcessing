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
	if (argc <= 1) {
		std::cout << "Input video path not provided." << std::endl;
		return -1;
	}
	VideoCapture input_cap(argv[1]);   
	
	// Check validity of target file
	if(!input_cap.isOpened()) {
		std::cout << "Input video not found." << std::endl;
		return -1;
	}
	
	namedWindow("output", WINDOW_AUTOSIZE);
	
	// Loop to read from input one frame at a time
	Mat frame;
	int count = 0;
	bool skipping = false;
	while(input_cap.read(frame)) {
			
		imshow("output", frame);

		if (skipping) {
			if (waitKey(0) == 27) {
				skipping = !skipping;
			}
		}

		// wait for ESC key to be pressed
		if(waitKey(30) == 27)
		{
			// break;
			skipping = !skipping;
		}
	}
	
	// free the capture objects from memory
	input_cap.release();
	
	return 1;
	
}