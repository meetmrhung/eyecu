%{
This function finds the centroid of a region that is colored yellow. this
 region shoould be the largest contigous region 

 Input:
        Image A where the pixels of interest are colored yellow: [255 255
           0]
        D: Connected region matrix
        darkPixels: coordinates of pixels that are part of connected
          regions.
        PUPIL_TO_BE_COLORED: label of the connected region that we
        identified as the pupil
       
% Output:
       % Centroid: [centerx, centery]
       % Image matrix A that now includes the lines to show centroid
%}
function [centroid A totalmass] = Centroid_Finding(A,D,darkPixels,PUPIL_TO_BE_COLORED)

totalmass = 0;
sumrmx = 0;
sumrmy = 0;

for ii = 1:size(darkPixels,1)
       if( D(darkPixels(ii,1),darkPixels(ii,2)) == PUPIL_TO_BE_COLORED)
            totalmass = totalmass+1;
            sumrmx = sumrmx+darkPixels(ii,1);
            sumrmy = sumrmy+darkPixels(ii,2);
        end
end


centroidy = sumrmx/totalmass;
centroidx = sumrmy/totalmass;
centroid = [centroidx centroidy];

% draws lines to show the centroid
x=floor(centroidx);
y=floor(centroidy);

if(x > 40 && x < size(A,2) && y > 40 && y < size(A,1))
    A(y, :, 1) = 0; A(y, :, 2) = 0; A(y, :, 3) = 255;    
    A(:, x, 1) = 0; A(:, x, 2) = 255; A(:, x, 3) = 0;
end






