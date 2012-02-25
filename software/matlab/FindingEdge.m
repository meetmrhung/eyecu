% This function finds the edge points of the yellow contigous region and
% colors them green. it also fits an ellipse give those edge points.

%Input:
    % A: Image data that has the region of interest colored yellow
    % I: Original image
% Output:
    % Ellipseprop: properties of the ellipse that was fitted
    % A: the input image with the edge points colored green and the red pixels from showin connected region taken away 
    

function [Ellipseprop A]  = FindingEdge(A,I, indx, indy)

% defines region of interest
xstart = 150; 
ystart = 20;
index = 1;
%edge points matrix
edge_points = [];

% colors the edge pixels green in the vertical direction starting from
% the bottom of the image.
for j = xstart:size(A,2)
    for i = size(A,1):-1:ystart
            if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3)==0)
            edge_points(index,1) = j;
            edge_points(index,2) = i;  
             A(i,j,1) = 0;
            A(i,j,2) = 255;
            A(i,j,3) = 0;
            index = index+1;    
            break;
            end
        end
end

for i = max(indy):-1:(max(indy)+min(indy))/2
    for j = min(indx):1:max(indx)
            if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3)==0)
            edge_points(index,1) = j;
            edge_points(index,2) = i;  
             A(i,j,1) = 0;
            A(i,j,2) = 255;
            A(i,j,3) = 0;
            index = index+1;    
            break;
            end
        end
end


for i = max(indy):-1:(max(indy)+min(indy))/2
    for j = max(indx):-1:min(indx)
            if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3)==0)
            edge_points(index,1) = j;
            edge_points(index,2) = i;  
             A(i,j,1) = 0;
            A(i,j,2) = 255;
            A(i,j,3) = 0;
            index = index+1;    
            break;
            end
        end
end



for i = ystart:size(A,1)
        for j = xstart:size(A,2)
            if (A(i,j,1) == 255 && A(i,j,2) == 0 && A(i,j,3)==0)
                A(i,j,1) = I(i,j,1);
                A(i,j,2) = I(i,j,2);
                A(i,j,3) = I(i,j,3);
            end
        end
end

%ax^2 + bxy + cy^2 +dx + ey + f = 0

Ellipseprop = EllipseFitByTaubin(edge_points);

% for j = min(indx):1:max(indx)
%     for i = min(indy):1:max(indy)
%         if abs(Ellipseprop(1)*j^2+Ellipseprop(2)*i*j+Ellipseprop(3)*i^2 + Ellipseprop(4)*j+Ellipseprop(5)*i+Ellipseprop(6))<0.005
%            A(i,j,1) = 0;
%             A(i,j,2) = 255;
%             A(i,j,3) = 0;
%         end
%     end
% end

