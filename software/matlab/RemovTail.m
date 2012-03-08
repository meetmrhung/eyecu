function [A]= RemovTail(A,I,D,darkPixels,PUPIL_TO_BE_COLORED)

k = 1;
C = zeros(512,2); 
for i  = 1:size(darkPixels,1)
            if( D(darkPixels(i,1),darkPixels(i,2)) == PUPIL_TO_BE_COLORED)
                C(k,1) = darkPixels(i,1);
                C(k,2) = darkPixels(i,2);
                k = k+1;
            else
                A(darkPixels(i,1), darkPixels(i,2),:) = I(darkPixels(i,1), darkPixels(i,2),:);
            end
end
 
[C(:,2) ind] = sort(C(:,2),'ascend');
C(:,1) = C(ind,1);
Unique_Matrix = unique(C(:,2));
histx_2 = [];
histx = zeros(length(Unique_Matrix), 1);
for i = 1:length(Unique_Matrix)
    histx(i,1) = length(find(C(:,2)== Unique_Matrix(i)));
    for k = 1:histx(i,1)
        histx_2 = [histx_2 histx(i,1)];
    end
end
limit = mean(histx(:,1))-0.5*std(histx(:,1));


 for j = 1:size(C,1)
    if (histx_2(j) < limit)
        A(C(j,1),C(j,2),:) = I(C(j,1),C(j,2),:);
    end
 end
 
 

[C(:,1) ind] = sort(C(:,1),'ascend');
C(:,2) = C(ind,2);
Unique_Matrix = unique(C(:,1));
histy_2 = [];
histy = zeros(length(Unique_Matrix), 1);
for i = 1:length(Unique_Matrix)
    histy(i,1) = length(find(C(:,1)== Unique_Matrix(i)));
    for k = 1:histy(i,1)
        histy_2 = [histx_2 histy(i,1)];
    end
end
limit = mean(histy(:,1))-0.5*std(histy(:,1));


Z = [];
k = 1;
 for j = 1:size(C,1)
    if (histy_2(j) < limit)
        A(C(j,1),C(j,2),:) = I(C(j,1),C(j,2),:);
    else
        Z(k,1) = C(j,1);
        Z(k,2) = C(j,2);
        k = k+1;
    end
 end
 
 
 
 
%% Needs work 
M = zeros(size(A,1),size(A,2));
for i = 1:size(A,1);
    for j = 1:size(A,2)
        if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3) == 0)
             M(i,j) = 1;      
        end      
    end
end


 for i = 1:size(A,1)
    val = find(M(i,:) == 1);
    for j = min(val):max(val)
        A(i,j,1) = 255;
        A(i,j,2) = 255;
        A(i,j,3) = 0;
    end
 end



