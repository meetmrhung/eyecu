#include <windows.h>
#include <stdio.h>
#include <highgui.h>

#include <stdlib.h>
#include <initguid.h>
#include "lmusbdll.h"
#include "luminary_guids.h"
#include <strsafe.h>

// Buffer size definitions.
//
//****************************************************************************
#define MAX_STRING_LEN 256
#define MAX_ENTRY_LEN 256
#define USB_BUFFER_LEN 930000

//****************************************************************************
//
// The build version number
//
//****************************************************************************
#define BLDVER "8555"

//****************************************************************************
//
// The number of bytes we read and write per transaction if in echo mode.
//
//****************************************************************************
#define ECHO_PACKET_SIZE 64

//****************************************************************************
//
// Buffer into which error messages are written.
//
//****************************************************************************
TCHAR g_pcErrorString[MAX_STRING_LEN];

//****************************************************************************
//
// The number of bytes transfered in the last measurement interval.
//
//****************************************************************************
ULONG g_ulByteCount = 0;

//****************************************************************************
//
// The total number of packets transfered.
//
//****************************************************************************
ULONG g_ulPacketCount = 0;

//****************************************************************************
//
// Return a string describing the supplied system error code.
//
// \param dwError is a Windows system error code.
//
// This function returns a pointer to a string describing the error code
// passed in the dwError parameter. If no description string can be found
// the string "Unknown" is returned.
//
// \return Returns a pointer to a string describing the error.
//
//****************************************************************************
LPTSTR GetSystemErrorString(DWORD dwError)
{
    DWORD dwRetcode;

    //
    // Ask Windows for the error message description.
    //
    dwRetcode = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, "%0", dwError, 0,
                              g_pcErrorString, MAX_STRING_LEN, NULL);

    if(dwRetcode == 0)
    {
        return((LPTSTR)L"Unknown");
    }
    else
    {
        //
        // Remove the trailing "\n\r" if present.
        //
        if(dwRetcode >= 2)
        {
            if(g_pcErrorString[dwRetcode - 2] == '\r')
            {
                g_pcErrorString[dwRetcode - 2] = '\0';
            }
        }

        return(g_pcErrorString);
    }
}



//****************************************************************************
//
// The main application entry function.
//
// \param None.
//
// \return Set the exit code to 0 of no errors cause the application to end
// or a non-zero value otherwise.
//
//****************************************************************************
int main(int argc, char *argv[])
{
    BOOL bResult;
    BOOL bDriverInstalled;
    char* szBuffer;
    ULONG ulWritten;
    ULONG ulRead;
    DWORD dwError;
    LMUSB_HANDLE hUSB;
	char* frame;
	char c;
	unsigned int i;

	IplImage *img;

	HANDLE hMapFile;
	LPCTSTR pBuf;
	TCHAR szName[]=TEXT("Local\\BeagleVideoTransfer");
	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		930000,
		szName);
	if(hMapFile == NULL)
	{
		printf("Can't open file map\n");
		return 1;
	}

	pBuf = (LPTSTR)MapViewOfFile(hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,0,
		930000);
	if(pBuf == NULL)
	{
		printf("Could not open file view\n");
		return 1;
	}

	frame = (char*)pBuf;
	cvNamedWindow("mainWin", CV_WINDOW_AUTOSIZE);
	cvMoveWindow("mainWin", 5,5);
	img = cvCreateImage(cvSize(640,480), 8, 3);

	szBuffer = (char*)malloc(sizeof(char)*USB_BUFFER_LEN);
    
    //
    // Find our USB device and prepare it for communication.
    //
    hUSB = InitializeDevice(BULK_VID, BULK_PID,
                            (LPGUID)&(GUID_DEVINTERFACE_STELLARIS_BULK),
                            &bDriverInstalled);
    if(hUSB)
    {
		szBuffer[0] = 1;
        while(1)
        {
            bResult = WriteUSBPacket(hUSB, szBuffer, 1, &ulWritten);
            if(!bResult)
            {
                //
                // We failed to write the data for some reason.
                //
                dwError = GetLastError();
                printf("Error %d (%S) writing to bulk OUT pipe.\n", dwError,
                       GetSystemErrorString(dwError));
				_getch();
				return 1;
            }
			//
            // We wrote data successfully so now read it back.
            //
            printf("Wrote %d bytes to the device\n",
                   ulWritten);

			dwError = ReadUSBPacket(hUSB, szBuffer, 921600, &ulRead,INFINITE, NULL);
				
			for(i = 0; i < ulRead; ++i)
			{
				//frame[i] = szBuffer[i];
				img->imageData[i] = szBuffer[i];
			}
			c = cvWaitKey(10);
			if(c == 27)
				break;
			cvShowImage("mainWin", img);
					
            if(dwError != ERROR_SUCCESS)
            {
                //
                // We failed to read from the device.
                //
                printf("Error %d (%S) reading from bulk IN pipe.\n", dwError,
                       GetSystemErrorString(dwError));
			}
		}
    }
    else
    {
        //
        // An error was reported while trying to connect to the device.
        //
        dwError = GetLastError();

        printf("\nUnable to initialize the Stellaris Bulk USB Device.\n");
        printf("Error code is %d (%S)\n\n", dwError, GetSystemErrorString(dwError));
        printf("Please make sure you have a Stellaris USB-enabled evaluation\n");
        printf("or development kit running the usb_dev_bulk example\n");
        printf("application connected to this system via the \"USB OTG\" or\n");
        printf("\"USB DEVICE\" connectors. Once the device is connected, run\n");
        printf("this application again.\n\n");

        printf("\nPress \"Enter\" to exit: ");
        fgets(szBuffer, MAX_STRING_LEN, stdin);
        printf("\n");
        return(2);
    }

    TerminateDevice(hUSB);

	UnmapViewOfFile(pBuf);
	CloseHandle(hMapFile);
	cvReleaseImage(&img);
	
    return(0);
}
