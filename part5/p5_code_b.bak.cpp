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
	
	namedWindow("output", WINDOW_AUTOSIZE);
	
	// vector<Vec3f> circle_locs_prev;
	Point circle_locs_prev[50];
	int circle_locs_prev_size = 0;
	vector<Vec3f> circle_locs_cur;
	const int MIN_LINE_MOVEMENT = 30;

	// Loop to read from input one frame at a time
	bool stepping = 0;
	const int TAU = 1;
	const int subtractTAU = 255 / TAU;
	Mat lastFrame, frame;
	input_cap.read(lastFrame);
	input_cap.read(frame);
	Mat motion_history_image = Mat::zeros(lastFrame.rows, lastFrame.cols, CV_8UC1);
	while (!frame.empty()) {
		
		Mat frame_gry;
		cvtColor(frame, frame_gry, COLOR_BGR2GRAY);

		Mat frameCopy;
		frame.copyTo(frameCopy);
		HoughCircles(
			frame_gry, circle_locs_cur,
			HOUGH_GRADIENT, 1,
			frame_gry.rows / 32, // minDist between center of circles
			100, 30,
			35, 45 // radius range
		);
		if (circle_locs_prev_size > 0 && circle_locs_cur.size() > 0) {
			
			// int mins_found_prev[circle_locs_prev.size()] = {0};
			Mat distMat = Mat::zeros(circle_locs_cur.size(), circle_locs_prev_size, CV_32SC1);
			for (int cur = 0; cur < circle_locs_cur.size(); ++cur) {
				Point center_c = Point(circle_locs_cur[cur][0], circle_locs_cur[cur][1]);
				for (int prev = 0; prev < circle_locs_prev_size; ++prev) {
					Point center_p = circle_locs_prev[prev];
					int xDiff = center_c.x - center_p.x;
					int yDiff = center_c.y - center_p.y;
					distMat.at<int>(cur, prev) = xDiff*xDiff + yDiff*yDiff;
				}
			}
			
			int mins_found_cur_match[circle_locs_cur.size()];
			for (int cur = 0; cur < circle_locs_cur.size(); ++cur) mins_found_cur_match[cur] = -1;
			for (int match = 0; match < circle_locs_cur.size(); ++match) {
				// Match each circle from current frame with circle from last frame
				// int min_cur = 0;
				// for (min_cur = 0; mins_found_cur_match[min_cur] >= 0; ++min_cur) {}
				int mins_found_cur[circle_locs_cur.size()];
				// for (int cur = 0; cur < circle_locs_cur.size(); ++cur) mins_found_cur[cur] = -1;
				
				for (int cur = 0; cur < distMat.rows; ++cur) {
					// Find closest circle from last frame for all current frame's circles (if they haven't yet been matched)
					if (mins_found_cur_match[cur] >= 0) continue;
					// if (mins_found_cur_match[cur] >= 0) { cout << "skipping cur: " << cur << endl; continue; }
					int min_prev = 0;
					// for (min_prev = 0; mins_found_cur_match[min_cur] == min_prev; ++min_prev) {}
					for (int prev = 0; prev < distMat.cols; ++prev) {
						// Find closest circle from last frame to the current circle (if they haven't yet been matched)
						// if (mins_found_cur_match[cur] == prev) continue;
						bool isAlreadyMatched = false;
						for (int match2 = 0; match2 < circle_locs_cur.size(); ++match2) {
							if (mins_found_cur_match[match2] == prev) {
								isAlreadyMatched = true;
								break;
							}
						}
						if (!isAlreadyMatched && distMat.at<int>(cur, min_prev) > distMat.at<int>(cur, prev)) {
							min_prev = prev;
						}
					}
					mins_found_cur[cur] = min_prev;
				}
				// for (int cur = 0; cur < distMat.rows; ++cur) {	
				// 	mins_found_cur_match[min_cur] = mins_found_cur[min_cur];
				// }
				int min_cur = 0;
				for (min_cur = 0; mins_found_cur_match[min_cur] >= 0 && min_cur < circle_locs_cur.size(); ++min_cur) {}
				for (int cur = min_cur+1; cur < circle_locs_cur.size(); ++cur) {
					if (mins_found_cur_match[cur] < 0 && distMat.at<int>(min_cur, mins_found_cur[min_cur]) > distMat.at<int>(cur, mins_found_cur[cur])) {
						min_cur = cur;
					}
				}
				mins_found_cur_match[min_cur] = mins_found_cur[min_cur];
			}
			// cout << distMat << endl;
			
			// cout << endl;
			// for (int cur = 0; cur < distMat.rows; ++cur) {
			// 	int min_prev = 0;
			// 	for (int prev = 1; prev < distMat.cols; ++prev) {
			// 		if (distMat.at<int>(cur, min_prev) > distMat.at<int>(cur, prev)) {
			// 			min_prev = prev;
			// 		}
			// 	}
			// 	mins_found_cur[cur] = min_prev;
			// }

			for (int cur = 0; cur < circle_locs_cur.size(); ++cur) {
				if (mins_found_cur_match[cur] >= 0 && distMat.at<int>(cur, mins_found_cur_match[cur]) > MIN_LINE_MOVEMENT) {
					Point center_c = Point(circle_locs_cur[cur][0], circle_locs_cur[cur][1]);
					int prev = mins_found_cur_match[cur];
					Point center_p = circle_locs_prev[prev];
					line(frameCopy, center_p, center_c, Scalar(0, 255, 0), 6, LINE_AA);
				}
				Point center = Point(circle_locs_cur[cur][0], circle_locs_cur[cur][1]);
				int radius = circle_locs_cur[cur][2];
				circle(frameCopy, center, radius, Scalar(0, 0, 255), 2, LINE_AA);
			}
		}
		circle_locs_prev_size = circle_locs_cur.size();
		for (int prev = 0; prev < circle_locs_prev_size; ++prev) {
			circle_locs_prev[prev].x = circle_locs_cur[prev][0];
			circle_locs_prev[prev].y = circle_locs_cur[prev][1];
		}
		imshow("FrameCopy", frameCopy);

		// Get new frame
		frame.copyTo(lastFrame);
		input_cap.read(frame);

		if (stepping) {
			// if (waitKey(0) == 27) {
			// 	imwrite("RollDiceChr.png", frame);
			// }
			if(waitKey(0) == 27)
			{
				// imwrite("RollDiceChr.png", img_chr);
				stepping = !stepping;
			}
		}

		// wait for ESC key to be pressed
		if(waitKey(30) == 27)
		{
			// imwrite("RollDiceChr.png", img_chr);
			stepping = !stepping;
		}
	}
	
	// free the capture objects from memory
	input_cap.release();
	
	return 1;
}