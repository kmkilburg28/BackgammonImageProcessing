function ret = label_components(I)
    [rows, cols] = size(I);
    compMat = zeros(rows+1,cols+1);
    equivTable = [];
    nextCompID = 1;
    for row = 1:rows
        for col = 1:cols
            if (I(row,col))
                if (compMat(row,col+1))
                    compMat(row+1,col+1) = compMat(row,col+1);
                    if (compMat(row+1,col) && compMat(row+1,col) ~= compMat(row,col+1))
                        % missing equivalency table addition
                        %equivTable(compMat(row,col+1)) = [equivTable(compMat(row,col+1)),compMat(row+1,col)];
                        equiv = [compMat(row,col+1),compMat(row+1,col)];
                        if (equiv(1) > equiv(2))
                            equiv = [equiv(2),equiv(1)];
                        end
                        equivIndex = find(equiv(1));
                        inTable = 0;
                        for i = 1:size(equivTable)
                            if (equivTable(i,1) == equiv(1) && equivTable(i,2) == equiv(2)) || ...
                                    (equivTable(i,1) == equiv(2) && equivTable(i,2) == equiv(1))
                                inTable = 1;
                                break;
                            end
                        end
                        if (~inTable)
                            equivTable = [equivTable; equiv];
                        end
                    end
                elseif (compMat(row+1,col))
                    compMat(row+1,col+1) = compMat(row+1,col);
                else
                    compMat(row+1,col+1) = nextCompID;
                    nextCompID = nextCompID + 1;
                end
            end
        end
    end
    compMat = compMat(2:end,2:end);
    
    largestCompID = max(equivTable,[],"all");
    if (length(largestCompID) <= 0)
        ret = zeros(rows, cols);
        return;
    end
    newEquivTable = zeros(largestCompID); % last col holds cur length of row
    for i = 1:length(newEquivTable)
        equivs = equivTable(:,1) == i;
        equivs = equivTable(equivs,2);
        
        for (equivInd = 1:size(equivs))
            newEquivTable(i,end) = newEquivTable(i,end) + 1;
            newEquivTable(i,newEquivTable(i,end)) = equivs(equivInd);
        end
    end
    equivTable = newEquivTable;
    %{
    for equivTableInd = 1:size(equivTable)
        if (equivTable(equivTableInd,end) > 0)
            for equivRowInd = 1:length(1:equivTable(equivTableInd,end))
                nextEquivInRow = equivTable(equivTableInd, equivRowInd);
                if (equivTable(nextEquivInRow,end) > 0)
                    recEquivs = get_rec_connected_comps(equivTable(nextEquivInRow,1:equivTable(nextEquivInRow,end)),equivTable)
                    for recEquivInd = 1:length(recEquivs)
                        recEquiv = recEquivs(recEquivInd);
%                         alreadyIn = equivTable(equivTableInd,1:equivTable(equivTableInd,end))
%                         ismember(recEquiv,equivTable(equivTableInd,1:equivTable(equivTableInd,end)))
                        if (~ismember(recEquiv,equivTable(equivTableInd,1:equivTable(equivTableInd,end))))
                            equivTable(equivTableInd,end) = equivTable(equivTableInd,end) + 1;
                            equivTable(equivTableInd,equivTable(equivTableInd,end)) = recEquiv;
                        end
                    end
                end
            end
        end
    end
    %}
    changed = 1;
    while changed
        changed = 0;
    for equivTableInd = 1:size(equivTable)
        if (equivTable(equivTableInd,end) > 0)
            equivRowInd = 1;
            while equivRowInd <= equivTable(equivTableInd,end)
                nextEquivInRow = equivTable(equivTableInd, equivRowInd);
                if (equivTable(nextEquivInRow,end) > 0)
                    for anotherEquivInd = 1:equivTable(nextEquivInRow,end)
                        anotherEquiv = equivTable(nextEquivInRow,anotherEquivInd);
                        if (~ismember(anotherEquiv,equivTable(equivTableInd,:)))
                            equivTable(equivTableInd,end) = equivTable(equivTableInd,end) + 1;
                            equivTable(equivTableInd,equivTable(equivTableInd,end)) = anotherEquiv;
%                             changed = 1
                        end
                    end
                end
                equivTable(nextEquivInRow,:) = zeros(size(equivTable(nextEquivInRow,:)));
                for subEquivTableInd = (equivTableInd+1):size(equivTable)
                %for subEquivTableInd = 1:size(equivTable)
                    if (subEquivTableInd ~= equivTableInd && ismember(nextEquivInRow,equivTable(subEquivTableInd,:)) && ~ismember(subEquivTableInd,equivTable(equivTableInd,:)))
                        equivTable(equivTableInd,end) = equivTable(equivTableInd,end) + 1;
                        equivTable(equivTableInd,equivTable(equivTableInd,end)) = subEquivTableInd;
%                         changed = 1;
                    end
                end
                equivRowInd = equivRowInd + 1;
            end
        end
    end
    end
%     equivTable
    
    % Simplifed Equivalence Table
    newEquivTable = zeros(largestCompID,1);        
    uniqueID = 1;
    for equivTableInd = 1:size(equivTable)
        if (equivTable(equivTableInd,end) > 0)
            newEquivTable(equivTableInd) = uniqueID;
            for equivInd = 1:equivTable(equivTableInd,end)
                newEquivTable(equivTable(equivTableInd,equivInd)) = uniqueID;
            end
            uniqueID = uniqueID + 1;
        end
    end
    equivTable = newEquivTable;
    
    % Simplify Component Matrix
    for (row = 1:rows)
        for (col = 1:cols)
            if (compMat(row, col) > 0)
                compMat(row, col) = equivTable(compMat(row, col));
            end
        end
    end
    
%     size(equivTable);
%     compMat
    ret = compMat;
    % Missing equivalency table simplification
end