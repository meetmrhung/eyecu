%% this function compares the area of connected region to ref pupil area
%% Input: 
        % RefArea: reference pupil area found from calibration
        % PixelCount: Area of the candidates for the pupil with
        % the area being in descending order
%% Output:
        % Direction: in which direction, threshold should be changed,
            % Direction is negative for decrease in threshold, positive for
            % increase in threshold
        % Pupil_Connected: 0 if a connected region does not meet the
            % area requirement, otherwise it is not zero. 
        
function [Pupil_Connected Direction] = CompareRefArea(refArea,PixelCount)
S = size(PixelCount,1);
Pupil_Connected = zeros(S,1);
Direction  = 0;

for i = 1:S
     if( PixelCount(i) > 0.85*refArea &&  PixelCount(i)<1.5*refArea)
            if (refArea ~= PixelCount(i))
                Pupil_Connected(i)=PixelCount(i)-refArea;
            else
                Pupil_Connected(i) = 1;    
            end
     end
end

if (sum (Pupil_Connected) == 0) %% check to see if any region met the requirement. 
    Direction = (PixelCount(1)-refArea)/(abs(PixelCount(1)- refArea)); 
end



