function [A indx indy]= RemovTail(A,I)
xstart = 30;
ystart = 30;
pixelcount = 0;
histx = zeros(1,size(A,2));
for j = xstart:size(A,2)
    for i = ystart:size(A,1)
        if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3) == 0)
            pixelcount = pixelcount+1;            
        end      
    end
    histx(j) = pixelcount;
    pixelcount = 0;
end

sumval = 0;
for i = 1:length(histx)
sumval = sumval + histx(i)*i;
end
data_mean = sumval/sum(histx);
data_std = std(histx);
limit = histx(floor(data_mean))-3*data_std;
indx = find(histx >0);


 for j = xstart:size(A,2)
    if (histx(j) < limit)
         for i = ystart:size(A,1)
             if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3) == 0)
                 A(i,j,:) = I(i,j,:);
             end
         end
    end
 end
 
 
 %%%%%
 C = zeros(size(A,1),size(A,2));
 pixelcount = 0;
 histy = zeros(1,size(A,1));
 for i = ystart:size(A,1);
    for j = xstart:size(A,2)
        if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3) == 0)
            pixelcount = pixelcount+1;            
        end      
    end
    histy(i) = pixelcount;
    pixelcount = 0;
end

sumval = 0;
for i = 1:length(histy)
sumval = sumval + histy(i)*i;
end
data_mean = sumval/sum(histy);
data_std = std(histy);
limit = histy(floor(data_mean))-3*data_std;


 for i = ystart:size(A,1)
    if (histy(i) < limit)
         for j = xstart:size(A,2)
             if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3) == 0)
                 A(i,j,:) = I(i,j,:);
             end
         end
    end
 end
 
 indy = find(histy >0);

for i = ystart:size(A,1);
    for j = xstart:size(A,2)
        if (A(i,j,1) == 255 && A(i,j,2) == 255 && A(i,j,3) == 0)
             C(i,j) = 1;      
        end      
    end
    histy(i) = pixelcount;
    pixelcount = 0;
end


 for i = ystart:size(A,1)
    val = find(C(i,:) == 1);
    for j = min(val):max(val)
        A(i,j,1) = 255;
        A(i,j,2) = 255;
        A(i,j,3) = 0;
    end
 end
         




