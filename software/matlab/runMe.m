%  Demonstration of thresholding and connected region finding

clc; clear all;close all

if(exist('mov') == 0)
    fprintf('Reading video...');
    %vid = VideoReader('vlc-record-2012-02-26-11h29m12s-dshow___-.avi');
    vid = VideoReader('Nick_Far.avi');
    nFrames = vid.NumberOfFrames;
    vidHeight = vid.Height;
    vidWidth = vid.Width;
    nFrames = 100;

    mov(1:nFrames) = struct('cdata', zeros(vidHeight, vidWidth, 3, 'uint8'), 'colormap', []);
    for k = 1 : nFrames
        mov(k).cdata = read(vid, 0+k);
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
myObj = VideoWriter('Nick_Far_Proc.avi');
myObj.FrameRate = 30;
open(myObj);
%h = waitbar(0, 'Creating Video');
tic
refArea = 260;   %% For Nick_Far dataset
%refArea  = 800;  
RefCentroid = [268.6042 222.0208];
global DirOutputVec ;
DirOutputVec= [];          % Direction Output Vector

for m = 1:nFrames
    Dir1 = 0;
    threshold = 51;
    cAdaptations = 1;
    IsBlink = 0;
    NextFrame = 1;
    while(NextFrame)
        %waitbar(m/nFrames, h, sprintf('Creating Video: %d%%', int16(100*m/nFrames)));
        A = mov(1,m).cdata;
        I = A;
        B = A(:,:,1);
        B_G = A(:,:,2);
       
        darkPixels = [];
        for i = ystart:yfinish
            for j = xstart:xfinish
                if(B(i,j) < threshold)
                    A(i,j,1) = 255;
                    A(i,j,2) = 0;
                    A(i,j,3) = 0;
                    darkPixels = [darkPixels; [i j]];
                end
            end
        end


        [A PixelCount sortIndex darkPixels D] = connectionMatrix(A,darkPixels,I);
        [A darkPixels D] = Complete_PUPIL(60, darkPixels, D, A, I, sortIndex);      % This is the part of the algorithm that tries to take care of specular reflection
        if (length(PixelCount ~= 0))   %% Check to see if any region with the threshold value is found.
            [PupilConnected Direction] = CompareRefArea(refArea,PixelCount);
              
            if (Direction == 0) % if there was a region that met the area requirement.
                [A IsPUPIL PUPIL_TO_BE_COLORED Direction] = Check_Aspect_Ratio(darkPixels, A, D, sortIndex,PupilConnected, Direction); 
                if (IsPUPIL == 1) 
                   NextFrame = 0; 
                end
            end
            
            % This part deals with the possibility of being in an infinite
            % loop (i.e alternating between Direction = 2 and Direction =
            % -2
            if (Dir1 == -Direction )
                Direction = Direction/2;
            end
            if (abs(Dir1)<1 && Dir1 ~= 0)          
                Direction = abs(Dir1)*Direction/2;
            end
            Dir1 = Direction;
        
            threshold = threshold - 2*Direction;
            cAdaptations = cAdaptations + 1;
        
            if(cAdaptations > 10)
                IsBlink = 1;    % User has blinked
                NextFrame = 0;
            end
        else
                IsBlink = 1; 
                NextFrame = 0;
        end
    end
    
    
   if (IsBlink == 0)
        [A] = RemovTail2(A,I,D,darkPixels,PUPIL_TO_BE_COLORED);
        [centroid A totalmass] = Centroid_Finding(A,D,darkPixels,PUPIL_TO_BE_COLORED);
   else
        A = I;  
   end
   
   

    if (IsBlink)
        Output  = -3;
    else
        [A Output] = GiveDirection(A,RefCentroid,centroid);  
    end
    DirOutputVec = CountConsecFrames(DirOutputVec, Output);
 
    writeVideo(myObj,A);
    %print(gcf, '-djpeg', ['./' sprintf('%d',m) '.jpg']);
    fprintf('Finished Processing %d image \n',m);
    
end
close(myObj);
fprintf('Done.\n');
toc