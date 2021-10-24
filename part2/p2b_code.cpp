#include <iostream>
#include <opencv2/opencv.hpp>
#include "label_components.h"

using namespace cv;
using namespace std;

Mat thinObjectsToCenterOfMass(Mat img);

int main(int argc, char** argv) {

	const int numSearches = 3;
	const char* searchFiles[numSearches] = {
		"p2_image1.png",
		"p2_image2.png",
		"p2_image3.png"
	};
	const char* imageFiles[numSearches] = {
		"p2_image1_result_opencv.png",
		"p2_image2_result_opencv.png",
		"p2_image3_result_opencv.png"
	};

	const int numDice = 6;
	const char* diceFiles[numDice] = {
		"../sample_dice_images/1_white.png",
		"../sample_dice_images/2_white.png",
		"../sample_dice_images/3_white.png",
		"../sample_dice_images/4_white.png",
		"../sample_dice_images/5_white.png",
		"../sample_dice_images/6_white.png"
	};
	for (int searchInd = 0; searchInd < numSearches; ++searchInd) {

		Mat img_color, img_gry, img_bw;
		img_color = cv::imread(searchFiles[searchInd], IMREAD_COLOR);

		if (img_color.empty()) {
			std::cout << "Input 1 is not a valid image file" << std::endl;
			return -1;
		}
		cvtColor(img_color, img_gry, COLOR_BGR2GRAY);
		threshold(img_gry, img_bw, 10, 255, THRESH_BINARY_INV);
		Mat SESquare = 255 * Mat::ones(3, 3, CV_8UC1);
		dilate(img_bw, img_bw, SESquare);

		for (int diceInd = numDice-1; diceInd >= 0; --diceInd) {
			Mat dice_gry, dice_bw;
			dice_gry = imread(diceFiles[diceInd], IMREAD_GRAYSCALE);

			if (dice_gry.empty()) {
				cout << "Dice file " << diceInd << " is not a valid image file" << endl;
				return -1;
			}

			// Resize die to be on same resolution scale
			// 1.875, 1.7222
			resize(dice_gry, dice_gry, Size(dice_gry.rows * 1.8, dice_gry.cols * 1.8), 0, 0, INTER_CUBIC);
			threshold(dice_gry, dice_bw, 10, 255, THRESH_BINARY_INV);

			Mat SEDisk = getStructuringElement(MORPH_ELLIPSE, Size(7, 7));
			erode(dice_bw, dice_bw, SEDisk);

			Mat locatedDieMat;
			erode(img_bw, locatedDieMat, dice_bw,
				Point(-1, -1), 1, BORDER_CONSTANT, Scalar(0, 0, 0)); // Need to specify the border color otherwise some corners are picked up as dice

			dilate(locatedDieMat, locatedDieMat, 255*Mat::ones(10, 10, CV_8UC1));

			Mat thinnedMat = thinObjectsToCenterOfMass(locatedDieMat);
			Mat dieSpaceMat;
			Mat SEdieSpace = 255*Mat::ones(100, 100, CV_8UC1);
			dilate(thinnedMat, dieSpaceMat, SEdieSpace);

			cv::bitwise_not(img_bw - dieSpaceMat, img_bw);
			cv::bitwise_not(img_bw, img_bw);

			for (int row = 0; row < thinnedMat.rows; ++row) {
				for (int col = 0; col < thinnedMat.cols; ++col) {
					if (thinnedMat.at<uint8_t>(row, col) > 0) {
						string text = to_string(diceInd + 1);
						HersheyFonts fontFace = FONT_HERSHEY_COMPLEX;
						int fontScale = 4;
						int thickness = 4;
						int baseline = 0;
						Scalar fontColor = Scalar(0, 255, 0);
						Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
						putText(img_color, to_string(diceInd+1), Point2i(col - textSize.width/2, row + textSize.height/2),
						 fontFace, fontScale, fontColor, thickness, LINE_AA);
					}
				}
			}
		}

		// imshow("Colored Image", img_color);
		// waitKey(0);
		// destroyAllWindows();

		imwrite(imageFiles[searchInd], img_color);
	}

	return 0;
}

Mat thinObjectsToCenterOfMass(Mat img) {
	Mat labelledMat = label_components(img);

	double _min, max;
	minMaxLoc(labelledMat, &_min, &max);

	// Row: {rowSum, colSum, area}
	Mat centerOfMassMat = Mat::zeros(max, 3, CV_32SC1);
	for (int row = 0; row < labelledMat.rows; ++row) {
		for (int col = 0; col < labelledMat.cols; ++col) {
			ushort id;
			if ((id = labelledMat.at<ushort>(row, col)) > 0) {
				centerOfMassMat.at<int>(id-1, 0) += row;
				centerOfMassMat.at<int>(id-1, 1) += col;
				++centerOfMassMat.at<int>(id-1, 2);
			}
		}
	}
	Mat thinnedMat = Mat::zeros(labelledMat.rows, labelledMat.cols, CV_8UC1);
	for (int id = 0; id < max; ++id) {
		float area = (float)centerOfMassMat.at<int>(id, 2);
		int xOrigin = (int)(centerOfMassMat.at<int>(id, 1) / area);
		int yOrigin = (int)(centerOfMassMat.at<int>(id, 0) / area);
		thinnedMat.at<uint8_t>(yOrigin, xOrigin) = 255;
	}

	return thinnedMat;
}