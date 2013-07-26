#ifndef APTINA_H
#define APTINA_H

#include <stdint.h>

#include "apbase.h"

class Aptina
{
public:
    Aptina();
    ~Aptina();

    void initialize(const char* initFile = NULL);
    bool grabFrame(uint8_t **imageBuf);




    ap_u32 getWidth() const { return mWidth; }
    ap_u32 getHeight() const { return mHeight; }
    ap_u32 getBufferSize() const { return mBufferSize; }

private:
    AP_HANDLE mHandle;
    ap_u32 mWidth;
    ap_u32 mHeight;
    ap_u32 mBufferSize;
    uint8_t * mImageBuf;
};

#endif
