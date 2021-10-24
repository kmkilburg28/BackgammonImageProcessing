function thinnedMat = thinObjectsToCenterOfMass(img)
    labelledMat = label_components(img);

	uniqueIDs = max(labelledMat,[],"all");

    [rows, cols] = size(labelledMat);
	% Row: {rowSum, colSum, area}
	centerOfMassMat = zeros(uniqueIDs, 3);
	for (row = 1:rows)
		for (col = 1:cols)
			id = labelledMat(row, col);
			if (id > 0)
				centerOfMassMat(id, 1) = centerOfMassMat(id, 1) + row;
				centerOfMassMat(id, 2) = centerOfMassMat(id, 2) + col;
				centerOfMassMat(id, 3) = centerOfMassMat(id, 3) + 1;
            end
        end
    end
	thinnedMat = zeros(rows, cols);
	for (id = 1:uniqueIDs)
		area = centerOfMassMat(id, 3);
		xOrigin = round(centerOfMassMat(id, 2) / area);
		yOrigin = round(centerOfMassMat(id, 1) / area);
		thinnedMat(yOrigin, xOrigin) = 255;
    end
end