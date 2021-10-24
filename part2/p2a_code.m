searchFiles = [
	"p2_image1.png", ...
	"p2_image2.png", ...
	"p2_image3.png"
];
imageFiles = [
	"p2_image1_result_matlab.png", ...
	"p2_image2_result_matlab.png", ...
	"p2_image3_result_matlab.png"
];
diceFiles = [
	"../sample_dice_images/1_white.png", ...
	"../sample_dice_images/2_white.png", ...
	"../sample_dice_images/3_white.png", ...
	"../sample_dice_images/4_white.png", ...
	"../sample_dice_images/5_white.png", ...
	"../sample_dice_images/6_white.png"
];

for (i = 1:searchFiles.length())
	img_color = imread(searchFiles(i));
    img_gry = im2gray(img_color);
    img_bw = ~imbinarize(img_gry, 10/255);
    
    SESquare = ones(3, 3);
	img_bw = imdilate(img_bw, SESquare);
    [rows, cols] = size(img_bw);
    
    
    for (diceInd = length(diceFiles):-1:1)
		dice_color = imread(diceFiles(diceInd));
        dice_gry = im2gray(dice_color);
        
        % resize(dice_gry, dice_gry, Size(dice_gry.rows * 0.87, dice_gry.cols * 0.87), 0, 0, INTER_CUBIC);
		% 1.875, 1.7222
		dice_gry = imresize(dice_gry, 1.8);
        dice_bw = ~imbinarize(dice_gry, 10/255);

		SEDisk = strel('disk', 5);
		dice_bw = imerode(dice_bw, SEDisk);

        % Create new matrix with larger, controlled border
        % Need to specify the border otherwise some corners are picked up as dice
        [dice_rows, dice_cols] = size(dice_bw);
        padded_img_bw = zeros(rows+2*dice_rows, cols+2*dice_cols);
        padded_img_bw(dice_rows+1:dice_rows+rows, dice_cols+1:dice_cols+cols) = img_bw;
        
        padded_locatedDieMat = imerode(padded_img_bw, dice_bw);
        locatedDieMat = padded_locatedDieMat(dice_rows+1:dice_rows+rows, dice_cols+1:dice_cols+cols);

		locatedDieMat = imdilate(locatedDieMat, 255*ones(10, 10));
%         diceFiles(diceInd)
        
        thinnedMat = thinObjectsToCenterOfMass(locatedDieMat);
		SEdieSpace = ones(100, 100);
		dieSpaceMat = imdilate(thinnedMat, SEdieSpace);
        
        img_bw = ~(~img_bw - dieSpaceMat);
		for (row = 1:rows)
			for (col = 1:cols)
				if (thinnedMat(row, col) > 0)
                    img_color = insertText(img_color, [col-60, row-90], num2str(diceInd), 'FontSize', 100, 'TextColor', 'green', 'BoxOpacity', 0.0);
                end
            end
        end
    end
%     imshow(img_color);
%     pause(1);
    
    imwrite(img_color, imageFiles(i));
end