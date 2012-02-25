function [A maxVal pixelCount sortIndex D] = connectionMatrix(A,darkPixels)
%  Colors the largested connected region of A
%  maxVal is the size of the region

%  Initialize
xDim = size(A,1);
yDim = size(A,2);
global D;                   %  Record cluster locations
D = zeros(xDim, yDim);
cLabel = 1;
labelSize = [];
pixelCount = 0;
%  Go through the entire image, and fill regions with red pixels
darkPixelsLen = size(darkPixels,1);
for i = 1:darkPixelsLen
    if( A(darkPixels(i,1),darkPixels(i,2),1) == 255 && A(darkPixels(i,1),darkPixels(i,2),2) == 0 && A(darkPixels(i,1),darkPixels(i,2),3) == 0 && D(darkPixels(i,1),darkPixels(i,2)) == 0)
        labelSize = [labelSize; fillRegion(darkPixels(i,1),darkPixels(i,2),1,A,cLabel)];
        cLabel = cLabel + 1;
    end
end

[maxVal maxIndex] = max(labelSize);
[sortVal sortIndex] = sort(labelSize,'descend');

%  Color the region based on D
for i = 1:darkPixelsLen
    if( D(darkPixels(i,1),darkPixels(i,2)) ~= maxIndex ) 
       % D(darkPixels(i,1),darkPixels(i,2)) = 0;
    else
        %A(darkPixels(i,1),darkPixels(i,2),2) = 255;
        %D(darkPixels(i,1),darkPixels(i,2)) = 1;
        pixelCount = pixelCount + 1;
    end
end
