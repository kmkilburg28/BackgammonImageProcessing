// label_components.cpp

#include <opencv2/opencv.hpp>

class Node {
	public:
		ushort id;
		Node* prev;
		Node* nextRow;
		Node* nextCol;

		Node(ushort i_id, Node* i_prev) {
			id = i_id;
			prev = i_prev;
			nextRow = NULL;
			nextCol = NULL;
		}
};


ushort findMinEquiv(ushort oldID, Node* equivTableHead) {
	Node* rowNode = equivTableHead->nextRow;
	while (rowNode != NULL) {
		Node* colNode = rowNode->nextCol;
		while (colNode != NULL) {
			if (oldID == colNode->id) {
				return rowNode->id;
			}
			colNode = colNode->nextCol;
		}
		rowNode = rowNode->nextRow;
	}
	std::cout << "ERROR: Did not find " << oldID << " in the equivTable!" << std::endl;
	return -1;
}

cv::Mat label_components(cv::Mat image) {
	int rows = image.rows;
	int cols = image.cols;


	cv::Mat compMat = cv::Mat::zeros(rows+1, cols+1, CV_16UC1);
	ushort nextCompID = 1;
	Node* equivTableHead = new Node(0, NULL);
	Node* equivTableTail = equivTableHead;
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			if (image.at<uint8_t>(row, col)) {
				if (compMat.at<ushort>(row, col+1)) {
					compMat.at<ushort>(row+1, col+1) = compMat.at<ushort>(row, col+1);
					ushort aboveID = compMat.at<ushort>(row, col+1);
					ushort leftID = compMat.at<ushort>(row+1, col);
					if (leftID && aboveID != leftID) {
						// Equivalent
						Node* aboveNode = NULL;
						Node* aboveNodeRow = NULL;
						Node* leftNode = NULL;
						Node* leftNodeRow = NULL;
						Node* rowNode = equivTableHead->nextRow;
						while (rowNode != NULL) {
							Node* colNode = rowNode;
							while (colNode != NULL) {
								if (colNode->id == aboveID || colNode->id == leftID) {
									if (colNode->id == aboveID) {
										aboveNode = colNode;
										aboveNodeRow = rowNode;
									}
									else {
										leftNode = colNode;
										leftNodeRow = rowNode;
									}
									if (aboveNode != NULL && leftNode != NULL) {
										if (aboveNodeRow != leftNodeRow) {
											Node* smallerRow;
											Node* largerRow;
											if (aboveNodeRow->id <= leftNodeRow->id) {
												smallerRow = aboveNodeRow;
												largerRow = leftNodeRow;
											}
											else {
												smallerRow = aboveNodeRow;
												largerRow = leftNodeRow;
											}

											// Remove larger row from row list
											largerRow->prev->nextRow = largerRow->nextRow;
											if (largerRow != equivTableTail) {
												largerRow->nextRow->prev = largerRow->prev;
											}
											else {
												equivTableTail = largerRow->prev;
											}

											// Place larger row at end of smaller row
											Node* endOfSmallerRow = smallerRow;
											while (endOfSmallerRow->nextCol != NULL) {
												endOfSmallerRow = endOfSmallerRow->nextCol;
											}
											endOfSmallerRow->nextCol = largerRow;
											largerRow->prev = endOfSmallerRow;
											largerRow->nextRow = NULL;
										}
										// else they're already equivalent
										goto resolvedEquivs;
									}
								}
								colNode = colNode->nextCol;
							}
							rowNode = rowNode->nextRow;
						}
					}
				}
				else if (compMat.at<ushort>(row+1, col)) {
					compMat.at<ushort>(row+1, col+1) = compMat.at<ushort>(row+1, col);
				}
				else
				{
					compMat.at<ushort>(row+1, col+1) = nextCompID;
					Node* nextNodeID = new Node(nextCompID, equivTableTail);
					equivTableTail->nextRow = nextNodeID;
					equivTableTail = nextNodeID;
					++nextCompID;
				}
			}
			resolvedEquivs: true;
		}
	}
	cv::Mat retMat;
	compMat(cv::Range(1, compMat.rows), cv::Range(1, compMat.cols)).copyTo(retMat);

	nextCompID = 1;
	Node* rowNode = equivTableHead->nextRow;
	while (rowNode != NULL) {
		Node* newRowHead = new Node(nextCompID, rowNode->prev);
		newRowHead->prev->nextRow = newRowHead;
		rowNode->prev = newRowHead;
		newRowHead->nextCol = rowNode;
		newRowHead->nextRow = rowNode->nextRow;
		rowNode->nextRow = NULL;
		if (newRowHead->nextRow != NULL) {
			newRowHead->nextRow->prev = newRowHead;
		}
		++nextCompID;
		rowNode = newRowHead->nextRow;
	}

	// Really should optimize this but it should be good enough
	// For every image component, search through equivTable and assign equivalent ID
	for (int row = 0; row < rows; row++)
	{
		for (int col = 0; col < cols; col++)
		{
			ushort oldID = retMat.at<ushort>(row, col);
			if (oldID > 0) {
				retMat.at<ushort>(row, col) = findMinEquiv(oldID, equivTableHead);
			}
		}
	}

	return retMat;
}