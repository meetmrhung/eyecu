function [A PixelCount sortIndex darkPixels D] = connectionMatrix(A,darkPixels,I)
%  Colors the largested connected region of A
%  maxVal is the size of the region

%  Initialize
xDim = size(A,1);
yDim = size(A,2);
global D;                   %  Record cluster locations
D = zeros(xDim, yDim);
cLabel = 1;
labelSize = [];

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
PixelCount = zeros(length(sortIndex),1);
for i = 1:darkPixelsLen
    for  k = 1:length(sortIndex)
        if( D(darkPixels(i,1),darkPixels(i,2)) == sortIndex(k) ) 
            PixelCount(k) = PixelCount(k)+1;
        end
    end
end

