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
	VideoCapture input_cap("p4_video1.avi");   
	
	// Check validity of target file
	if(!input_cap.isOpened()) {
		std::cout << "Input video not found." << std::endl;
		return -1;
	}

	VideoWriter output_cap("p4a_result1.avi", 
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
	
		Mat img_gry;
		cvtColor(frame, img_gry, COLOR_BGR2GRAY);
		medianBlur(img_gry, img_gry, 5);

		vector<Vec3f> circle_locs;
		const int MAX_RADIUS = 45;
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
			Scalar markColor = (sum > 0 && 255*rgb(2) / sum > 100) ?
				Scalar(0,255,0) : Scalar(255,0,0);
			circle(frame, center, MAX_RADIUS, markColor, 3, LINE_AA);
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