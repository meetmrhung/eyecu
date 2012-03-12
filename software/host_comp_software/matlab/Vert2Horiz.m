% Input:
    %Connect: determines of which connectivity Region they are: 1,2,3
    % Compares vertical to horizontal size of the connected region.
    % numConnect: number of connected regions
    % Connect_Matrix: shows connectivity 
    
    function [] = Vert2Horiz(Connect_Matrix,numConnect)
    IndConn = zeros(numConnect,2);
    xstart = 50;
    ystart = 150;
    maxval = zeros(numConnect,1);
    minval = zeros(numConnect,1);
    for k = 0:numConnect  
        for i = ystart: size(Connect_Matrix,1)
            for j = xstart:size(Connect_Matrix,2)
                if (C(i,j) == k)
                    if (minval(k) > j)
                        minval(k) = j;
                    end
                    if (maxval(k) < j )
                    maxval(k) = j;
                    end
                end                
            end
        end
    end
    rangeX = maxval - minval;
    maxval = zeros(numConnect,1);
    minval = zeros(numConnect,1);

    for k = 0:numConnect  
        for j = xstart: size(Connect_Matrix,2)
            for i = ystart:size(Connect_Matrix,1)
                if (C(i,j) == k)
                    if (minval(k) > i)
                        minval(k) = i;
                    end
                    if (maxval(k) < i )
                    maxval(k) = i;
                    end
                end                
            end
        end
    end
    rangeY = maxval-minval;
    
    
    
    