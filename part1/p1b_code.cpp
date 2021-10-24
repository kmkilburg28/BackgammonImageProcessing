#include <iostream>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

int main(int argc, char** argv) {

	const int numSearches = 3;
	const char* searchFiles[numSearches] = {
		"p1_image1.png",
		"p1_image2.png",
		"p1_image3.png"
	};
	const char* imageFiles[numSearches] = {
		"p1_image1_result_opencv.png",
		"p1_image2_result_opencv.png",
		"p1_image3_result_opencv.png"
	};
	for (int searchNum = 0; searchNum < numSearches; ++searchNum) {

		cv::Mat img_color, img_gry, img_bw;
		img_color = cv::imread(searchFiles[searchNum], IMREAD_COLOR); // flag = 0 => CV_LOAD_IMAGE_GRAYSCALE

		if (img_color.empty()) {
			std::cout << "Input 1 is not a valid image file" << std::endl;
			return -1;
		}
		cvtColor(img_color, img_gry, COLOR_BGR2GRAY);

		vector<Vec3f> circle_locs;
		Mat colorCopy;
		img_color.copyTo(colorCopy);
		HoughCircles(
			img_gry, circle_locs,
			HOUGH_GRADIENT, 1,
			img_gry.rows / 32, // minDist between center of circles
			100, 30,
			35, 45 // radius range
		);
		for (int i = 0; i < circle_locs.size(); i++) {
			// cout << circle_locs[i] << endl;
			Point center = Point(circle_locs[i][0], circle_locs[i][1]);
			// circle(copy, center, 1, Scalar(0,100,100), 3, LINE_AA);
			int radius = circle_locs[i][2];
			circle(colorCopy, center, radius, Scalar(0,255,0), 3, LINE_AA);
		}

		// cv::imshow("Show Image", copy);
		// cv::waitKey(0);
		// cv::destroyAllWindows();

		cv::imwrite(imageFiles[searchNum], colorCopy);
	}

	return 0;
}