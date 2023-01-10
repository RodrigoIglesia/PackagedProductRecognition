/* Computer vision main process.
.. 
*/

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/dnn.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <vector>
#include <string>
#include <tchar.h>
#include <strsafe.h>
#include <bitset>

#include "SapClassBasic.h"
#include "PlcComm.h"
#include "NeuralNetworkClassifier.h"

using namespace cv;
using namespace dnn;
using namespace std;

// Restore deprecated function warnings with Visual Studio 2005
#if defined(_MSC_VER) && _MSC_VER >= 1400
#pragma warning(default: 4995)
#endif

#define CAMERA_LINK_SERVER_NAME_PREFIX "CameraLink_"


// Transfer callback function.
void XferCallback(SapXferCallbackInfo* pInfo)
{
    SapView* pView = (SapView*)pInfo->GetContext();
    //pView->Show();
    SapBuffer* Buffer_View = (pView->GetBuffer());
}

int main(int argc, char* argv[])
{
    PlcComm plcOcj; // Invoque plc comm class.
    NeuralNetworkClassifier NNClassifier;  // Invoque Neural Network class.

    /* Camera config vars.*/
    UINT32 acqDeviceNumber;
    char acqServerName[CORSERVER_MAX_STRLEN];
    const char* configFilename = new char[MAX_PATH];

    configFilename = "D_Genie_C1600.ccf"; // Camera config file.

    SapAcquisition Acq;
    SapAcqDevice AcqDevice;
    SapBufferWithTrash Buffers;
    SapTransfer AcqToBuf = SapAcqToBuf(&Acq, &Buffers);
    SapTransfer AcqDeviceToBuf = SapAcqDeviceToBuf(&AcqDevice, &Buffers);
    SapTransfer* Xfer = NULL;
    SapView View;

    SapFormat sapFormat;

    /* Named Pipe vars.*/
    HANDLE hPipe;
    DWORD dwWrite, dwWrite2, dwWrite3;

    // Create the Pipe.
    hPipe = CreateNamedPipe(L"\\\\.\\pipe\\Pipe", PIPE_ACCESS_DUPLEX, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1,
        1024 * 16,
        1024 * 16,
        NMPWAIT_USE_DEFAULT_WAIT,
        NULL);

    /* ACQ SYSTEM INFO*/

    // Number of connected devices.
    int serverCount = SapManager::GetServerCount();
    if (serverCount == 0)
    {
        printf("No device found!\n");
        return FALSE;
    }

    // Name and index of connected device.
    BOOL serverFound = FALSE;
    BOOL cameraFound = FALSE;
    int serverIndex = 1;

    if (SapManager::GetResourceCount(serverIndex, SapManager::ResourceAcqDevice) != 0)
    {
        SapManager::GetServerName(serverIndex, acqServerName, sizeof(acqServerName));

        printf("Camera connected %d: %s\n", serverIndex, acqServerName);
        cameraFound = TRUE;
    }

    // At least there must be 1 cammera connected.
    if (!serverFound && !cameraFound)
    {
        printf("Not cammera found!\n");
        return FALSE;
    }

    // Resources connected.
    int cameraIndex = 0;
    char cameraName[CORPRM_GETSIZE(CORACQ_PRM_LABEL)];
    SapManager::GetResourceName(acqServerName, SapManager::ResourceAcqDevice, cameraIndex, cameraName, sizeof(cameraName));
    printf("Resources connected %d: %s\n", cameraIndex + 1, cameraName);

    /* Sapera objects */
    SapLocation loc(acqServerName, 0);

    AcqDevice = SapAcqDevice(loc, configFilename);
    Buffers = SapBufferWithTrash(2, &AcqDevice);
    View = SapView(&Buffers, SapHwndAutomatic);
    AcqDeviceToBuf = SapAcqDeviceToBuf(&AcqDevice, &Buffers, XferCallback, &View);
    Xfer = &AcqDeviceToBuf;

    // Acq object.
    if (!AcqDevice.Create())
        goto FreeHandles;

    // Buffer object.
    if (!Buffers.Create())
        goto FreeHandles;

    // Transfer object.
    if (Xfer && !Xfer->Create())
        goto FreeHandles;

    // View object
    //if (!View.Create())
    //    goto FreeHandles;

    /* ACQUISITION*/
    while (1)
    {
        // If client connects Pipe (Interfaze sends message) -> Start vision process
        printf("Waiting for client to connect...\n");
        while (hPipe != INVALID_HANDLE_VALUE)
        {
            if (ConnectNamedPipe(hPipe, NULL) != FALSE)
            {
                printf("\nClient Connected.\n");

                /* Obtain box code and sensor state from PLC*/
                int numCaja = plcOcj.GetPlcNum();
                char fdCaja = plcOcj.GetPlcFd();
                delete plcOcj.MyClient;

                if (fdCaja == '1')
                {
                    printf("Box in sensor.\n");

                    if (numCaja != 0 && numCaja != 15)
                        // Box code must be differet to 0 and ? (values in ASCII).
                        plcOcj.contCaja += 1;
                    else
                        plcOcj.contCaja = 0;

                    if (plcOcj.contCaja == 1)
                    {
                        // Box code ir read.
                        printf("New box, code: %d\n", numCaja);

                        // Obtain image.
                        Xfer->Snap(1);
                        Xfer->Wait(10);
                        if (!Xfer->Wait(5000))
                            printf("Could not obtain image.\n");

                        sapFormat = Buffers.GetFormat();

                        int OpenCV_Type = 0;
                        OpenCV_Type = CV_8UC4;
                        
                        if (sapFormat != SapFormatUnknown)
                        {
                            auto start = chrono::high_resolution_clock::now();
                            // Export to OpenCV Mat object using SapBuffer data directly
                            void* pBuf = NULL;
                            Buffers.GetAddress(&pBuf);
                            // Load buffer content to Mat image.
                            Mat image(Buffers.GetHeight(), Buffers.GetWidth(), OpenCV_Type, pBuf);

                            Buffers.ReleaseAddress(&pBuf);

                            // Make a prediction with loaded image.
                            string predClass = NNClassifier.predictClass(image);
                            // Show result.
                            cout << predClass << endl;

                            // Save frst image acquiredx.
                            string saveDir = "Imgs\\img";
                            saveDir = saveDir + to_string(plcOcj.contImg) + "_" + to_string(numCaja) + "_" + predClass;
                            saveDir = saveDir + ".bmp";
                            imwrite(saveDir, NNClassifier.imageRGB);

                            auto stop = chrono::high_resolution_clock::now();

                            auto predTime = chrono::duration_cast<chrono::seconds>(stop - start);

                            cout << "Processing time: " << predTime.count() << " s" << endl;

                            /* Send RGB image to Interface*/
                            printf("Sending image to interface...\n");

                            // Encode image to jpeg.
                            vector<uchar> imgBuff; //buffer for coding
                            vector<int> param(2);

                            param[0] = IMWRITE_JPEG_QUALITY;
                            param[1] = 95; //default(95) 0-100

                            imencode(".jpg", NNClassifier.imageRGB, imgBuff, param);
                            printf("Image data size: %l bytes (%l kB)\n", imgBuff.size(), (int)(imgBuff.size() / 1024));

                            if (WriteFile(hPipe, imgBuff.data(), imgBuff.size(), &dwWrite, NULL) < 0)
                            {
                                printf("Error while sending image through Pipe.\n");
                            }
                            Sleep(100);

                            /* Send box code to interface.*/
                            printf("Sending box code to interface...\n");

                            // Encode box code (int) as byte array.
                            vector<uchar> numBuff(4);
                            for (int i = 0; i < 4; i++)
                            {
                                numBuff[3 - i] = (numCaja >> (i * 8));
                            }

                            if (WriteFile(hPipe, numBuff.data(), numBuff.size(), &dwWrite2, NULL) < 0)
                            {
                                printf("Error while sending box code through Pipe.\n");
                            }
                            Sleep(100);

                            /* Send prediction result to interface.*/
                            printf("Sending prediction to interface...\n");

                            if (WriteFile(hPipe, predClass.c_str(), strlen(predClass.c_str()), &dwWrite3, NULL) < 0)
                            {
                                printf("Error while sending prediction through Pipe.\n");
                            }
                        }
                        plcOcj.contImg++;
                    }
                }
            }
            DisconnectNamedPipe(hPipe);
        }
    }
FreeHandles:
    printf("Press any key to terminate\n");
    CorGetch();

    //unregister the acquisition callback
    Acq.UnregisterCallback();

    // Destroy view object
    //if (!View.Destroy()) return FALSE;

    // Destroy transfer object
    if (Xfer && *Xfer && !Xfer->Destroy()) return FALSE;

    // Destroy buffer object
    if (!Buffers.Destroy()) return FALSE;

    // Destroy acquisition object
    if (!Acq.Destroy()) return FALSE;

    // Destroy acquisition object
    if (!AcqDevice.Destroy()) return FALSE;
    return 0;
}
