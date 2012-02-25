% This function finds the centroid of a region that is colored yellow. this
% region shoould be the largest contigous region 

% Input:
       % Image A where the pixels of interest are colored yellow: [255 255
       % 0]
% Output:
       % Centroid: [centerx, centery]
       % Image matrix A that now includes the lines to show centroid
function [centroid A] = Centroid_Finding(A)

totalmass = 0;
sumrmx = 0;
sumrmy = 0;

for (ii = 1:size(A,1))
    for (jj = 1:size(A,2))
        if (A(ii,jj,1)== 255 && A(ii,jj,2) == 255 && A(ii,jj,3) == 0)
            totalmass = totalmass+1;
            sumrmx = sumrmx+ii;
            sumrmy = sumrmy+jj;
        end
    end
end

            
centroidy = sumrmx/totalmass;
centroidx = sumrmy/totalmass;
centroid = [centroidx centroidy];

% draws lines to show the centroid
x=floor(centroidx);
y=floor(centroidy);

if(x > 1 && x < size(A,2) && y > 1 && y < size(A,1))
    A(y, :, 1) = 0; A(y, :, 2) = 0; A(y, :, 3) = 255;    
    A(:, x, 1) = 0; A(:, x, 2) = 255; A(:, x, 3) = 0;
end






