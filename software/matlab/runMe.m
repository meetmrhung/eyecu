%  Demonstration of thresholding and connected region finding

clc; close all; clear all;

if(exist('mov') == 0)
    fprintf('Reading video...');
    vid = VideoReader('Armeen_Far.avi');
    nFrames = vid.NumberOfFrames;
    vidHeight = vid.Height;
    vidWidth = vid.Width;
    nFrames = 10;


    mov(1:nFrames) = struct('cdata', zeros(vidHeight, vidWidth, 3, 'uint8'), 'colormap', []);
    for k = 1 : nFrames
        mov(k).cdata = read(vid, 2+k);
    end
    
    fprintf('Done\n');
end

%  Define region to process
xstart = 30;
ystart = 30;
xfinish = 640;
yfinish = 400;

adapt = [5 3 2];

fprintf('Creating video...');
myObj = VideoWriter('Armeen_Far_Proc.avi');
myObj.FrameRate = 30;
open(myObj);
%h = waitbar(0, 'Creating Video');

threshold = 31;
tic
for m = 1:nFrames
    
    cAdaptations = 1;
    while(true)
        %waitbar(m/nFrames, h, sprintf('Creating Video: %d%%', int16(100*m/nFrames)));
        A = mov(1,m).cdata;
        %B = rgb2gray(A);
        I = A;
        B = A(:,:,1);
        B_G = A(:,:,2);
        % 
         %figure
         %imshow(mov(1,m).cdata(:,:,1));
    %     figure
    %     imshow(mov(1,m).cdata(:,:,2));
    %     figure
    %     imshow(mov(1,m).cdata(:,:,3));
        %figure; imshow(B);
        %  Indicate processing region
        %A(ystart, :, 1) = 0; A(ystart, :, 2) = 0; A(ystart, :, 3) = 255;    
        %A(:, xstart, 1) = 0; A(:, xstart, 2) = 255; A(:, xstart, 3) = 0;

        %  Set pixels under threshold to red
        darkPixels = [];
        for i = ystart:yfinish
            for j = xstart:xfinish
                if(B(i,j) < threshold &&  B_G(i,j) <  threshold)
                    A(i,j,1) = 255;
                    A(i,j,2) = 0;
                    A(i,j,3) = 0;
                    darkPixels = [darkPixels; [i j]];
                end
            end
        end


        [A maxVal pixelCount sortIndex D] = connectionMatrix(A,darkPixels);
        [A IsBLink] = Check_Aspect_Ratio(D,sortIndex, 4, A);
        imshow(A);
        [A indx indy] = RemovTail(A,B);
   
        if( m == 1)
            refArea = pixelCount;
            break;
        end
        if( pixelCount < 0.85*refArea )
            threshold = threshold + 2;
        elseif( pixelCount > 1.15*refArea )
            threshold = threshold - 2;
        else
            break;
        end
        cAdaptations = cAdaptations + 1;
        if(cAdaptations > 10)
            break;
        end
    end
   [centroid A] = Centroid_Finding(A);
   if m == 1
       RefCentorid = centroid;
   else
       GiveDirection(A,RefCentorid, centroid);
   end
       
       
   figure; imshow(A);
   writeVideo(myObj,A);
    %print(gcf, '-djpeg', ['./' sprintf('%d',m) '.jpg']);
    fprintf('Finished Processing %d image \n',m);
    
end
close(myObj);
fprintf('Done.\n');
close(h);
toc