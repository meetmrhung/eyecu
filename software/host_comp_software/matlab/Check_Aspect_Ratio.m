%{
 This function finds the aspect ratio of connected regions. If the aspect
 ratio is approxiamtely (1), i.e the connected region is an ellipse
 Inputs:
        darkPixels: coordinates that are part of connected reggions
        A: processed Image
        D = matrix containing coordinates of the connected region
        sortIndex: the connected regions sorted from the largest to
            smallst
        PupilConnected: connected regions that have area equalling reference area has index zero. 
         
        
 Outputs:
         A: processed image with the largest connected region that
         satisfies the aspect ratio colored yellow.
         IsPUPIL : 1 if Pupil has been found, 0 otherwise
         PUPIL_TO_BE_COLORED: 
%}        
function [A IsPUPIL PUPIL_TO_BE_COLORED Direction] = Check_Aspect_Ratio(darkPixels, A, D, sortIndex,PupilConnected, Direction)


S = length(PupilConnected);


xmin = 640*ones(S,1);
xmax = zeros(S,1);

ymin = 480*ones(S,1);
ymax = zeros(S,1);

%% finding the minimum, maximum x and y location of a connected region that has met area requirement.
for z = 1:length(PupilConnected)
    if (PupilConnected(z) ~= 0)
        for i  = 1:size(darkPixels,1)
            if( D(darkPixels(i,1),darkPixels(i,2)) == sortIndex(z))
                if (ymin(z) > darkPixels(i,1))
                    ymin(z) = darkPixels(i,1);
                end
                if (ymax(z) < darkPixels(i,1))
                    ymax(z) = darkPixels(i,1);
                end
                if (xmin(z) > darkPixels(i,2))
                    xmin(z) = darkPixels(i,2);
                end
                if (xmax(z) < darkPixels(i,2))
                    xmax(z) = darkPixels(i,2);
                end    
            end
        end
    end
end

%% Computing the aspect ratio
yaspect = ymax-ymin;
xaspect = xmax-xmin;
aspRatio =yaspect./xaspect;


PupilLabel = 10*ones(S,1);

for i = 1:S
    if (aspRatio(i)>.6 && aspRatio(i)<2 && aspRatio(i) ~= 480/640)           % aspect raio approximately 1
        PupilLabel(i) = abs(aspRatio(i)-1);                                  % gives us a sense of how close to circular it is. 
        break;
    end
end

[val ind] = min(PupilLabel);                                                 % finds the connected region with aspect ratio that is closes to circle
PUPIL_TO_BE_COLORED = sortIndex(ind);   

if (length(ind) ~= 0 && val ~=10)                                            % value = 10 for regions that dont meet the aspect ratio requirement
    IsPUPIL = 1;                                                             % have found the pupil
     for i  = 1:size(darkPixels,1)
            if( D(darkPixels(i,1),darkPixels(i,2)) == PUPIL_TO_BE_COLORED && darkPixels(i,1)>40 && darkPixels(i,2)>40)
                A(darkPixels(i,1), darkPixels(i,2),1) = 255;
                A(darkPixels(i,1), darkPixels(i,2),2) = 255;
                A(darkPixels(i,1), darkPixels(i,2),3) = 0;
            end
     end
else
    IsPUPIL = 0;                                                             % Pupil is not found. 
    ind = find(PupilConnected ~= 0);
    [val ind2] = min(abs(PupilConnected(ind)));                              % Connected region with area closest to reference Area selected.  
    Direction  = PupilConnected(ind(ind2))/abs(PupilConnected(ind(ind2)));   
end
    

    
            
