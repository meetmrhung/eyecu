% This function assigns a direction given the reference centroid and other
% centroids

% Input:
       % A: Image Data
       % Centroid: [centroidx centroidy]
       % RefCentorid: [centroidx, centroidy]
% Output:
       % the Image matrix
 function [A,Output] =  GiveDirection(A,RefCentroid,Centroid)
         if (abs(Centroid(2)-RefCentroid(2))> abs(Centroid(1)-RefCentroid(1))>0 )
           if (Centroid(2)>RefCentroid(2) && Centroid(2)-RefCentroid(2) > 5)
               Output = -1;   %% down
           elseif(Centroid(2)<RefCentroid(2) && -Centroid(2)+RefCentroid(2) > 7)
               Output = 1; %% up
           else
               Output = 0; % no movement
           end
       else
           if (Centroid(1)>RefCentroid(1) && Centroid(1)-RefCentroid(1) > 7)
               Output = 2;      %% right
           elseif (Centroid(1)<RefCentroid(1) && -Centroid(1)+RefCentroid(1) > 15)
               Output = 3;      %% left
           else
               Output = 0;      % no movement
           end
       end
 end
%  C = I;
%  C(RefCentroid(2)-2:RefCentroid(2)+2,RefCentroid(1)-2:RefCentroid(1)+2,1) = 255;
%  C(RefCentroid(2)-2:RefCentroid(2)+2,RefCentroid(1)-2:RefCentroid(1)+2,1) = 0;
%  C(RefCentroid(2)-2:RefCentroid(2)+2,RefCentroid(1)-2:RefCentroid(1)+2,1) = 0;
%  hold on
% rectangle('Position', [RefCentroid(1)-15,RefCentroid(2)-7,25,14],'LineWidth',4,'LineStyle','--') 
%  
   
 