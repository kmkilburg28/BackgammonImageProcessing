// Headers
#include "label_components.h"
#include <opencv2/core/core.hpp> 
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

// compile command
// cl /EHsc play_vid_test.cpp /I D:\installs\opencv\opencv\build\include /link /LIBPATH:D:\installs\opencv\opencv\build\x64\vc15\lib opencv_world451.lib

Mat getObjectsInSize(Mat img, int min, int max);
Mat getObjectsCenterOfMass(Mat img);

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

	VideoWriter output_cap("p4b_result1.avi", 
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

	Mat SEtriangleLightDown = imread("triangleLightDown.png", IMREAD_COLOR);
	cvtColor(SEtriangleLightDown, SEtriangleLightDown, COLOR_BGR2GRAY);
	medianBlur(SEtriangleLightDown, SEtriangleLightDown, 5);
	threshold(SEtriangleLightDown, SEtriangleLightDown, 140, 255, THRESH_BINARY_INV);
	Mat SEtriangleLightUp;
	flip(SEtriangleLightDown, SEtriangleLightUp, 0); // flip over x-axis
	
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
		// Mark circles
		for (int i = 0; i < circle_locs.size(); i++) {
			Point center = Point(circle_locs[i][0], circle_locs[i][1]);
			Vec3b rgb = frame.at<Vec3b>(center.y, center.x);
			int sum = rgb(0) + rgb(1) + rgb(2);
			Scalar markColor = (sum > 0 && 255*rgb(2) / sum > 100) ?
				Scalar(0,255,0) : Scalar(255,0,0);
			circle(frame, center, MAX_RADIUS, markColor, 3, LINE_AA);
		}

		const int EPSILON = 20;
		// Create theshold with all triangles visible
		Mat img_bw;
		threshold(img_gry, img_bw, 140, 255, THRESH_BINARY_INV);
		// Make circles match triangle
		for (int circleInd = 0; circleInd < circle_locs.size(); circleInd++) {
			Point center = Point(circle_locs[circleInd][0], circle_locs[circleInd][1]);
			Mat SEcircle = 255*getStructuringElement(MORPH_ELLIPSE, Size(2*MAX_RADIUS, 2*MAX_RADIUS));
			Range rowRange = Range(center.y-MAX_RADIUS, center.y+MAX_RADIUS);
			Range colRange = Range(center.x-MAX_RADIUS, center.x+MAX_RADIUS);
			img_bw(rowRange, colRange);
			bitwise_or(img_bw(rowRange, colRange), SEcircle, img_bw(rowRange, colRange));
		}
		
		dilate(img_bw, img_bw, 255*Mat::ones(15, 15, CV_8UC1));

		// Isolate triangles
		Mat trianglesDown;
		erode(img_bw, trianglesDown, SEtriangleLightDown);
		Mat trianglesUp;
		erode(img_bw, trianglesUp, SEtriangleLightUp);
		Mat triangles;
		bitwise_or(trianglesDown, trianglesUp, triangles);

		// Remove fake edge triangles
		triangles = getObjectsInSize(triangles, 6000, 15000);
		// Get centers of triangles
		Mat triangleCenters = getObjectsCenterOfMass(triangles); // Note: another recalculation using label_components()
		
		// Create score storage for each triangle
		int numTriangles = triangleCenters.rows;
		int middleY = frame.rows / 2;
		char* triangleChars = (char*)calloc(numTriangles, sizeof(char));
		uint8_t* triangleCounts = (uint8_t*)calloc(numTriangles, sizeof(uint8_t));

		// For each triangle, count the number of circles within it and identify their color
		for (int triangleInd = 0; triangleInd < numTriangles; ++triangleInd) {
			int triangleX = triangleCenters.at<int>(triangleInd, 0);
			int triangleY = triangleCenters.at<int>(triangleInd, 1);
			for (int circleInd = 0; circleInd < circle_locs.size(); ++circleInd) {
				Point center = Point(circle_locs[circleInd][0], circle_locs[circleInd][1]);
				if (triangleX-EPSILON < center.x && center.x < triangleX+EPSILON) {
					bool isCircleTop = center.y < middleY;
					bool isTriangleTop = triangleY < middleY;
					if ((isCircleTop && isTriangleTop) || (!isCircleTop && !isTriangleTop)) {
						++*(triangleCounts+triangleInd);
						Vec3b rgb = frame.at<Vec3b>(center.y, center.x);
						int sum = rgb(0) + rgb(1) + rgb(2);
						*(triangleChars+triangleInd) = (sum > 0 && 255*rgb(2) / sum > 100) ? 'r' : 'w';
					}
				}
			}
		}
		// Write each triangles' count onto triangle center
		for (int triangleInd = 0; triangleInd < numTriangles; ++triangleInd) {
			
			int triangleX = triangleCenters.at<int>(triangleInd, 0);
			int triangleY = triangleCenters.at<int>(triangleInd, 1);
			char chainString[3];
			if (*(triangleChars+triangleInd) != 0) {
				chainString[0] = *(triangleChars+triangleInd);
				chainString[1] = (char)((uint8_t)'0' + *(triangleCounts+triangleInd)); // does not work for chainLength's over 9
				chainString[2] = '\0';
			}
			else {
				chainString[0] = (char)((uint8_t)'0' + *(triangleCounts+triangleInd)); // does not work for chainLength's over 9
				chainString[1] = '\0';
			}

			string text = string(chainString);
			HersheyFonts fontFace = FONT_HERSHEY_COMPLEX;
			int fontScale = 2;
			int thickness = 4;
			int baseline = 0;
			Scalar fontColor = Scalar(0, 255, 0);
			Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
			putText(frame, text, Point2i(triangleX - textSize.width/2, triangleY + textSize.height/2),
				fontFace, fontScale, fontColor, thickness, LINE_AA);
		}
		free(triangleChars);
		triangleChars = NULL;
		free(triangleCounts);
		triangleCounts = NULL;


		// // 2nd Solution: Identify stacked circles
		// CircleNode* redHead = new CircleNode(-1, -1, -1);
		// CircleNode* whiteHead = new CircleNode(-1, -1, -1);
		// 	Point center = Point(circle_locs[circleInd][0], circle_locs[circleInd][1]);
		// 	Vec3b rgb = frame.at<Vec3b>(center.y, center.x);
		// 	int sum = rgb(0) + rgb(1) + rgb(2);
		// 	Scalar markColor;
		// 	CircleNode* colorHead;
		// 	if (sum > 0 && 255*rgb(2) / sum > 100) { // found red piece
		// 		markColor = Scalar(0,255,0);
		// 		colorHead = redHead;
		// 	}
		// 	else {
		// 		markColor = Scalar(255,0,0);
		// 		colorHead = whiteHead;
		// 	}
		// 	CircleNode* circleIndNode = new CircleNode(circleInd, center.x, center.y);
		// 	CircleNode* curChain = colorHead;
		// 	bool matched = false;
		// 	while (curChain->nextChain) {
		// 		curChain = curChain->nextChain;
		// 		CircleNode* curNode = curChain;
		// 		while (curNode) {
		// 			if (curNode->x-EPSILON < center.x && center.x < curNode->x+EPSILON &&
		// 				(center.y-2*MAX_RADIUS < curNode->y && curNode->y < center.y+2*MAX_RADIUS))
		// 			{
		// 				CircleNode* nextConnected = curNode->nextConnected;
		// 				curNode->nextConnected = circleIndNode;
		// 				circleIndNode->prev = curNode;

		// 				circleIndNode->nextConnected = nextConnected;
		// 				if (nextConnected != NULL) {
		// 					nextConnected->prev = circleIndNode;
		// 				}
		// 				matched = true;
		// 			}
		// 			curNode = curNode->nextConnected;
		// 		}
		// 	}
		// 	if (!matched) {
		// 		curChain->nextChain = circleIndNode;
		// 		circleIndNode->prev = curChain;
		// 		matched = true;
		// 	}
		// 	circle(frame, center, MAX_RADIUS, markColor, 3, LINE_AA);
		// }

		// for (int colorInd = 0; colorInd < 2; ++colorInd) {
		// 	CircleNode* curChain = (colorInd == 0 ? redHead : whiteHead);
		// 	char colorChar = (colorInd == 0 ? 'r' : 'w');
		// 	while (curChain->nextChain) {
		// 		curChain = curChain->nextChain;
		// 		uint8_t chainLength = 0;
		// 		int sumX = 0;
		// 		int maxY = 0;
		// 		int minY = frame.rows;
		// 		CircleNode* curNode = curChain;
		// 		while (curNode != NULL) {
		// 			++chainLength;
		// 			sumX += curNode->x;
		// 			if (maxY < curNode->y) {
		// 				maxY = curNode->y;
		// 			}
		// 			if (minY > curNode->y) {
		// 				minY = curNode->y;
		// 			}
		// 			curNode = curNode->nextConnected;
		// 		}
		// 		int writeY = (maxY < frame.rows / 2) ? minY : maxY;
		// 		int writeX = sumX / chainLength;
		// 		char chainString[] = {colorChar, (char)((uint8_t)'0' + chainLength), '\0'}; // does not work for chainLength's over 9
		// 		string text = string(chainString);
		// 		HersheyFonts fontFace = FONT_HERSHEY_COMPLEX;
		// 		int fontScale = 4;
		// 		int thickness = 4;
		// 		int baseline = 0;
		// 		Scalar fontColor = Scalar(0, 255, 0);
		// 		Size textSize = getTextSize(text, fontFace, fontScale, thickness, &baseline);
		// 		putText(frame, text, Point2i(writeX - textSize.width/2, writeY + textSize.height/2),
		// 			fontFace, fontScale, fontColor, thickness, LINE_AA);
		// 	}
		// }

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

Mat getObjectsInSize(Mat img, int minSize, int maxSize) {
	Mat labelledMat = label_components(img);

	double _min, uniqueIDs;
	minMaxLoc(labelledMat, &_min, &uniqueIDs);

	// Row: {rowSum, colSum, area}
	Mat sizeMat = Mat::zeros(uniqueIDs, 1, CV_32SC1);
	int rows = labelledMat.rows;
	int cols = labelledMat.cols;
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			ushort id;
			if ((id = labelledMat.at<ushort>(row, col)) > 0) {
				++sizeMat.at<int>(id-1, 0);
			}
		}
	}
	Mat acceptedMat = Mat::zeros(rows, cols, CV_8UC1);
	for (int row = 0; row < rows; ++row) {
		for (int col = 0; col < cols; ++col) {
			ushort id;
			if ((id = labelledMat.at<ushort>(row, col)) > 0) {
				int size = sizeMat.at<int>(id-1, 0);
				if (minSize <= size && size <= maxSize) {
					acceptedMat.at<uint8_t>(row, col) = 255;
				}
			}
		}
	}

	return acceptedMat;
}

Mat getObjectsCenterOfMass(Mat img) {
	Mat labelledMat = label_components(img);

	double _min, max;
	minMaxLoc(labelledMat, &_min, &max);

	// Row: {rowSum, colSum, area}
	Mat centerOfMassMatBuilder = Mat::zeros(max, 3, CV_32SC1);
	for (int row = 0; row < labelledMat.rows; ++row) {
		for (int col = 0; col < labelledMat.cols; ++col) {
			ushort id;
			if ((id = labelledMat.at<ushort>(row, col)) > 0) {
				centerOfMassMatBuilder.at<int>(id-1, 0) += row;
				centerOfMassMatBuilder.at<int>(id-1, 1) += col;
				++centerOfMassMatBuilder.at<int>(id-1, 2);
			}
		}
	}
	Mat centerOfMassMat = Mat::zeros(max, 2, CV_32SC1);
	for (int id = 0; id < max; ++id) {
		float area = (float)centerOfMassMatBuilder.at<int>(id, 2);
		int xOrigin = (int)(centerOfMassMatBuilder.at<int>(id, 1) / area);
		int yOrigin = (int)(centerOfMassMatBuilder.at<int>(id, 0) / area);
		centerOfMassMat.at<int>(id, 0) = xOrigin;
		centerOfMassMat.at<int>(id, 1) = yOrigin;
	}

	return centerOfMassMat;
}