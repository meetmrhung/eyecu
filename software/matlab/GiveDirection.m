% This function assigns a direction given the reference centroid and other
% centroids

% Input:
       % A: Image Data
       % Centroid: [centroidx centroidy]
       % RefCentorid: [centroidx, centroidy]
% Output:
       % the Image matrix
 function [A] =  GiveDirection(A,RefCentroid,Centroid)
         if (abs(Centroid(2)-RefCentroid(2))> abs(Centroid(1)-RefCentroid(1))>0 )
           if (Centroid(2)> RefCentroid(2))
               disp(['down'])
           else
               disp(['up'])
           end
       else
           if (Centroid(1)>RefCentroid(1))
               disp(['right'])
           else
               disp(['left'])
           end
       end
   end