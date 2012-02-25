% This function finds the aspect ratio of connected regions. If the aspect
% ratio is approxiamtely (1), i.e the connected region is an ellipse
% Inputs:
        %D = matrix containing coordinates of the connected region
        %sortIndex: the connected regions sorted from the largest to
            %smallst
        % numConnected: number of connected regions considered
        % A: processed Image
        
% Outputs:
        % A: processed image with the largest connected region that
        % satisfies the aspect ratio colored yellow.

function [A] = YversusX(D, sortIndex, numConnected,A)


% Region that we are processing
xstart = 30;
ystart = 30;
C = zeros(size(D,1), size(D,2));


for k = 1:numConnected
    for i = ystart: size(D,1)
        for j = xstart: size(D,2)
            if D(i,j) == sortIndex(k)
                C(i,j) = k;             % holds matrix of the connected regions labeled
            end          
        end
    end
end


xmax = zeros(size(D,2),numConnected);
xmin = size(D,2)*ones(size(D,2),numConnected);

for i = ystart: size(D,1)
   for k = 1:numConnected
       val = find(C(i,:) == k);
       if length(val) ==0
           xmax(i,k) = 0;
           xmin(i,k) = size(D,2); 
       else
           xmax(i,k) = max(val);
           xmin(i,k) = min(val); 
       end
   end
end


ymax = zeros(size(D,1),numConnected);
ymin = size(D,1)*ones(size(D,1),numConnected);
for i = xstart: size(D,2)
   for k = 1:numConnected
       val = find(C(:,i) == k);
       if length(val) ==0
           ymax(i,k) = 0;
           ymin(i,k) = size(D,1);
       else
           ymax(i,k) = max(val);
           ymin(i,k) = min(val); 
       end
   end
end

yaspect = max(ymax)-min(ymin);
xaspect = max(xmax)-min(xmin);
aspRatio =yaspect./xaspect;

for i = 1:numConnected
    if (aspRatio(i)>1/2 && aspRatio(i)<2)           % aspect raio approximately 1
        PupilLabel = i;
        break;
    end
end

% Color the largest connected region that meets aspect ratio requirement
% yellow
 for i = ystart: size(D,1)
        for j = xstart: size(D,2)
            if C(i,j) == PupilLabel
                A(i,j,2) = 255;
            end          
        end
 end



