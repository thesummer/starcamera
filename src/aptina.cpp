#include <stdexcept>
#include <iostream>
using std::endl;
using std::cout;

#include "aptina.h"

Aptina::Aptina()
    :mHandle(NULL), mWidth(0), mHeight(0)
{
}

Aptina::~Aptina()
{
    delete [] mImageBuf;
    ap_Finalize();
}


void Aptina::initialize(const std::string initFile)
{
    if(ap_DeviceProbe(NULL) != AP_CAMERA_SUCCESS)
        throw std::runtime_error("Unable to either detect a sensor or find a matching SDAT file.");

    const char * fileName = initFile.empty() ? NULL : initFile.c_str();

    mHandle = ap_Create(0);
    if(mHandle == NULL)
        throw std::runtime_error("Camera initialization failed");

    int result = ap_LoadIniPreset(mHandle, fileName, "Reset");

    if( result != AP_INI_SUCCESS)
        throw std::runtime_error("Failed to load IniPreset");


    result = ap_LoadIniPreset(mHandle, fileName, "StarCam");

    if( result != AP_INI_SUCCESS)
        throw std::runtime_error("Failed to load IniPreset");


    ap_GetImageFormat(mHandle, &mWidth, &mHeight, NULL, 0);

    mBufferSize = ap_GrabFrame(mHandle, NULL, 0);

    mImageBuf = new uint8_t[mBufferSize];
}

bool Aptina::grabFrame(uint8_t **imageBuf)
{
    if(mHandle == NULL)
        throw std::runtime_error("Grab frame failed. Camera handle not initialized");

    ap_u32 numBytes = ap_GrabFrame(mHandle, mImageBuf, mBufferSize);
    ap_s32 result = ap_GetLastError();
    cout << numBytes << "\t" << result << endl;
    if(result != AP_CAMERA_SUCCESS)
        return false;

    *imageBuf = mImageBuf;
    return true;
}
