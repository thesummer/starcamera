#ifndef APTINA_H
#define APTINA_H

#include <stdint.h>
#include <string>

#include "apbase.h"

/*!
 \brief Class which provides the functionalities of the Aptina

 Internally handles the camera interface through the Apbase library

*/
class Aptina
{
public:
    /*!
    \brief Constructor

        Creates and uninitialized camera object
    */
    Aptina();

    /*!
     \brief Destructor
    */
    ~Aptina();

    /*!
     \brief Initializes the camera

     Sets up the Apbase interface, sets the camera registers
     and allocates memory for frames.

     \param initFile File which contains the camera register settings for the Apbase library
    */
    void initialize(const std::string initFile = NULL);

    /*!
     \brief Grabs a frame from the camera and writes it into the buffer

     The buffer size is determined from the initialize()-function.
     The formate is Bayer-12 in 2 bytes with leading 0s

     Note:
          At the moment no internal buffer handling is done, hence the function
          will overwrite the last frame. Therefore the caller must ensure that
          processing of the frame is finished.

     TODO:
          Use double or triple buffer to avoid side effects from above mentioned
          behaviour.

     \param imageBuf Will contain the address of the image buffer the frame was written to
     \return bool True if the frame was grabbed successfully.
    */
    bool grabFrame(uint8_t **imageBuf);




    /*!
     \brief Returns the number of columns of the frame

     \return ap_u32
    */
    ap_u32 getWidth() const { return mWidth; }

    /*!
     \brief Returns the number of rows of the frame

     \return ap_u32
    */
    ap_u32 getHeight() const { return mHeight; }

    /*!
     \brief Returns the length in bytes of the allocated image buffer

     \return ap_u32
    */
    ap_u32 getBufferSize() const { return mBufferSize; }

private:
    AP_HANDLE mHandle; /*!< Camera handle of the Apbase library */
    ap_u32 mWidth; /*!< Frame width with current camera settings*/
    ap_u32 mHeight; /*!< Frame height with current camera settings*/
    ap_u32 mBufferSize; /*!< Buffer size (in Bytes) with current camera settings*/
    uint8_t * mImageBuf; /*!< Image buffer*/
};

#endif
