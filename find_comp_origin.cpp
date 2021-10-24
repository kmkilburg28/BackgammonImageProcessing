// find_comp_origin.cpp

#include <opencv2/opencv.hpp>

cv::Point find_comp_origin(cv::Mat image) {
	int rows = image.rows;
	int cols = image.cols;

	double Area = cv::sum(image)[0];
	double xOrigin = 0;
	double yOrigin = 0;
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			uint8_t value = image.at<uint8_t>(row, col);
			xOrigin += col * value;
			yOrigin += row * value;
		}
	}
	if (Area == 0) {
		return cv::Point(-1, -1);
	}
	xOrigin /= Area;
	yOrigin /= Area;

	return cv::Point((int)xOrigin, (int)yOrigin);
}