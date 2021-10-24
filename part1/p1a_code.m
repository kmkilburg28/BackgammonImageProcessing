searchFiles = [
	"p1_image1.png", ...
	"p1_image2.png", ...
	"p1_image3.png"
];
imageFiles = [
	"p1_image1_result_matlab.png", ...
	"p1_image2_result_matlab.png", ...
	"p1_image3_result_matlab.png"
];

for (i = 1:searchFiles.length())
	img_color = imread(searchFiles(i));

	[centers, radii] = imfindcircles(img_color, [35 45], 'ObjectPolarity', 'dark',...
        'Sensitivity', 0.90, 'Method', 'twostage');
    
    %imshow(img_color);
    %h = viscircles(centers, radii);
    
    % Paste circles on image
    thickness = 3;
    thickness_mat = -floor(thickness/2):floor(thickness/2);
    thickness_len = length(thickness_mat);
    colored_area = ones(thickness_len, thickness_len, 3);
    for (row = 1:thickness_len)
        for (col = 1:thickness_len)
            colored_area(row, col, :) = [0 255 0];
        end
    end
    for (circleInd = 1:length(centers))
        theta = 0:360;
        cols = round(centers(circleInd, 1) + radii(circleInd)*sin(theta));
        rows = round(centers(circleInd, 2) + radii(circleInd)*cos(theta));
        
        for (radAngle = 1:length(theta))
            img_color(rows(radAngle)+thickness_mat, cols(radAngle)+thickness_mat, :) = colored_area;
        end
    end
    
    imwrite(img_color, imageFiles(i));
end
