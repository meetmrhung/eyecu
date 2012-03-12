function rSize = fillRegion(x, y, m, A, label)
%  Discover connected regions and record results to D
%  A = image matrix (RGB)
%  m = search radius
%  D = matrix containing coordinates of the connected region

global D;
rSize = 0;
xDim = size(A,1);
yDim = size(A,2);
pointQueue = [x;y];

while(length(pointQueue) ~= 0)
    %  Mark the current point, take it out of the queue
    lastEle = size(pointQueue,2);
    x = pointQueue(1, lastEle);
    y = pointQueue(2, lastEle);
    pointQueue(:,lastEle) = [];
    rSize = rSize + 1;
    D(x,y) = label;
    
    %  Add neighbors in a cross region of distance i
    for i = 1:m        
        if( (y+i <= yDim) && (D(x,y+i) == 0) && (A(x,y+i,1) == 255) && (A(x,y+i,2) == 0) && (A(x,y+i,3) == 0) )
            pointQueue = [pointQueue [x;y+i]];
        end

        if(x+i <= xDim && (D(x+i,y) == 0) && (A(x+i,y,1) == 255) && (A(x+i,y,2) == 0) && (A(x+i,y,3) == 0))
            pointQueue = [pointQueue [x+i;y]];
        end   

        if( (x-i >= 1) && (D(x-i,y) == 0) && (A(x-i,y,1) == 255) && (A(x-i,y,2) == 0) && (A(x-i,y,3) == 0))
            pointQueue = [pointQueue [x-i;y]];
        end


        if( (y - i >= 1) && (D(x,y-i) == 0) && (A(x,y-i,1) == 255) && (A(x,y-i,2) == 0) && (A(x,y-i,3) == 0))
            pointQueue = [pointQueue [x;y-i]];
        end
    end
end

    