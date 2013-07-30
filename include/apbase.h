//**************************************************************************************
// Copyright 2012 Aptina Imaging Corporation. All rights reserved.
//
//
// No permission to use, copy, modify, or distribute this software and/or
// its documentation for any purpose has been granted by Aptina Imaging Corporation.
// If any such permission has been granted ( by separate agreement ), it
// is required that the above copyright notice appear in all copies and
// that both that copyright notice and this permission notice appear in
// supporting documentation, and that the name of Aptina Imaging Corporation or any
// of its trademarks may not be used in advertising or publicity pertaining
// to distribution of the software without specific, written prior permission.
//
//
// This software and any associated documentation are provided "AS IS" and
// without warranty of any kind. APTINA IMAGING CORPORATION EXPRESSLY DISCLAIMS
// ALL WARRANTIES EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO, NONINFRINGEMENT
// OF THIRD PARTY RIGHTS, AND ANY IMPLIED WARRANTIES OF MERCHANTABILITY OR FITNESS
// FOR A PARTICULAR PURPOSE. APTINA DOES NOT WARRANT THAT THE FUNCTIONS CONTAINED
// IN THIS SOFTWARE WILL MEET YOUR REQUIREMENTS, OR THAT THE OPERATION OF THIS SOFTWARE
// WILL BE UNINTERRUPTED OR ERROR-FREE. FURTHERMORE, APTINA DOES NOT WARRANT OR
// MAKE ANY REPRESENTATIONS REGARDING THE USE OR THE RESULTS OF THE USE OF ANY
// ACCOMPANYING DOCUMENTATION IN TERMS OF ITS CORRECTNESS, ACCURACY, RELIABILITY,
// OR OTHERWISE.
//*************************************************************************************/

#ifndef _APBASE_H_
#define _APBASE_H_

#ifdef _WIN32
#ifdef APBASE_EXPORTS
#define APBASE_API  __declspec(dllexport)
#else
#define APBASE_API  __declspec(dllimport)
#ifndef APBASE_NO_LIB
#pragma comment(lib, "apbase.lib")
#endif
#endif
#define APBASE_DECL  __stdcall
#else
#define APBASE_API
#define APBASE_DECL
#endif


#ifdef __cplusplus
extern "C" {
#endif

typedef void *  AP_HANDLE;

//  Symbols from Midlib
#include "midlib2.h"
//  Aliases with ap_ or AP_
typedef mi_u8       ap_u8;
typedef mi_u16      ap_u16;
typedef mi_u32      ap_u32;
typedef mi_s8       ap_s8;
typedef mi_s16      ap_s16;
typedef mi_s32      ap_s32;
typedef mi_intptr   ap_intptr;
typedef mi_u64      ap_u64;
typedef mi_s64      ap_s64;
enum  
{
     AP_CAMERA_SUCCESS          = MI_CAMERA_SUCCESS         ,
     AP_CAMERA_ERROR            = MI_CAMERA_ERROR           ,
     AP_GRAB_FRAME_ERROR        = MI_GRAB_FRAME_ERROR       ,
     AP_NOT_ENOUGH_DATA_ERROR   = MI_NOT_ENOUGH_DATA_ERROR  ,
     AP_EOF_MARKER_ERROR        = MI_EOF_MARKER_ERROR       ,
     AP_BUFFER_SIZE_ERROR       = MI_BUFFER_SIZE_ERROR      ,
     AP_SENSOR_FILE_PARSE_ERROR = MI_SENSOR_FILE_PARSE_ERROR,
     AP_SENSOR_DOES_NOT_MATCH   = MI_SENSOR_DOES_NOT_MATCH  ,
     AP_SENSOR_NOT_INITIALIZED  = MI_SENSOR_NOT_INITIALIZED ,
     AP_SENSOR_NOT_SUPPORTED    = MI_SENSOR_NOT_SUPPORTED   ,
     AP_I2C_BIT_ERROR           = MI_I2C_BIT_ERROR          ,
     AP_I2C_NACK_ERROR          = MI_I2C_NACK_ERROR         ,
     AP_I2C_TIMEOUT             = MI_I2C_TIMEOUT            ,
     AP_CAMERA_TIMEOUT          = MI_CAMERA_TIMEOUT         ,
     AP_CAMERA_NOT_SUPPORTED    = MI_CAMERA_NOT_SUPPORTED   ,
     AP_PARSE_SUCCESS           = MI_PARSE_SUCCESS          ,
     AP_DUPLICATE_DESC_ERROR    = MI_DUPLICATE_DESC_ERROR   ,
     AP_PARSE_FILE_ERROR        = MI_PARSE_FILE_ERROR       ,
     AP_PARSE_REG_ERROR         = MI_PARSE_REG_ERROR        ,
     AP_UKNOWN_SECTION_ERROR    = MI_UKNOWN_SECTION_ERROR   ,
     AP_CHIP_DESC_ERROR         = MI_CHIP_DESC_ERROR        ,
     AP_PARSE_ADDR_SPACE_ERROR  = MI_PARSE_ADDR_SPACE_ERROR ,
     AP_INI_SUCCESS             = MI_INI_SUCCESS            ,
     AP_INI_KEY_NOT_SUPPORTED   = MI_INI_KEY_NOT_SUPPORTED  ,
     AP_INI_LOAD_ERROR          = MI_INI_LOAD_ERROR         ,
     AP_INI_POLLREG_TIMEOUT     = MI_INI_POLLREG_TIMEOUT    ,
     AP_INI_HANDLED_SUCCESS     = MI_INI_HANDLED_SUCCESS    ,
     AP_INI_HANDLED_ERROR       = MI_INI_HANDLED_ERROR      ,
     AP_INI_NOT_HANDLED         = MI_INI_NOT_HANDLED        ,
};


//   Register write side-effects flags
enum
{
    AP_FLAG_OK                    = 0x0000,    //  No side-effect
    AP_FLAG_REALLOC               = 0x0001,    //  Image buffer size may change
    AP_FLAG_PAUSE                 = 0x0002,    //  Sensor may stop streaming
    AP_FLAG_RESUME                = 0x0004,    //  Sensor may resume streaming
    AP_FLAG_NOT_SUPPORTED         = 0x0008,    //  Selecting a mode that is not supported by the demo system
    AP_FLAG_ILLEGAL_REG_COMBO     = 0x0010,    //  The new value creates an invalid combination with some other register(s)
    AP_FLAG_ILLEGAL_REG_VALUE     = 0x0020,    //  The new value is not supported by the device
    AP_FLAG_REGISTER_RESET        = 0x0040,    //  Many other register values will change (reset or state change)
    AP_FLAG_CLOCK_FREQUENCY       = 0x0080,    //  The clock frequency will change (this is a PLL register or clock divider)
    AP_FLAG_REG_LIST_CHANGED      = 0x0100,    //  The set of camera registers will change
    AP_FLAG_NOT_ACCESSIBLE        = 0x0200,    //  Register is not accessible (standby)
    AP_FLAG_READONLY              = 0x0400,    //  Writing a read-only register
    AP_FLAG_WRITEONLY             = 0x0800,    //  Reading a write-only register
};

//  I/O Log Flags
enum
{
    AP_NO_ERROR_LOG               = MI_NO_ERROR_LOG ,
    AP_ERROR_SEVERE               = MI_ERROR_SEVERE ,
    AP_ERROR_MINOR                = MI_ERROR_MINOR  ,
    AP_ALL_ERRORS                 = MI_ALL_ERRORS   ,
    AP_LOG                        = MI_LOG          ,
    AP_LOG_SHIP                   = MI_LOG_SHIP     ,
    AP_LOG_USB                    = MI_LOG_USB      ,
    AP_LOG_DEBUG                  = MI_LOG_DEBUG    ,
    AP_ALL_LOGS                   = MI_ALL_LOGS     ,
    AP_ERROR_LOG_ALL              = MI_ERROR_LOG_ALL,
};

//  Register or bitfield accessibility
enum
{
    AP_RW_READONLY                = MI_RW_READONLY ,
    AP_RW_READWRITE               = MI_RW_READWRITE,
    AP_RW_WRITEONLY               = MI_RW_WRITEONLY,
};

APBASE_API const char * APBASE_DECL
ap_Home();

APBASE_API const char * APBASE_DECL
ap_SensorData();

APBASE_API ap_s32 APBASE_DECL
ap_DeviceProbe(const char *szSensorDataDirOrFile);

APBASE_API ap_s32 APBASE_DECL
ap_DeviceProbeDll(const char *szSensorDataDirOrFile,
                  const char *szTransportDll);

APBASE_API int APBASE_DECL
ap_NumCameras();

APBASE_API void APBASE_DECL
ap_CancelProbe();

APBASE_API void APBASE_DECL
ap_Finalize();

APBASE_API ap_s32 APBASE_DECL
ap_GetLastError();

APBASE_API ap_s32 APBASE_DECL
ap_GetLastSideEffects();

APBASE_API void APBASE_DECL
ap_OpenIoLog(ap_u32 nLogFlags, const char *szFilename);

APBASE_API const char * APBASE_DECL
ap_GetIoLogFilename();

APBASE_API void APBASE_DECL
ap_IoLogMsg(const char *szText);

APBASE_API void APBASE_DECL
ap_IoLogDebugMsg(const char *szText, const char *szSource, const char *szFunc, ap_u32 nLine);

APBASE_API void APBASE_DECL
ap_CloseIoLog();

APBASE_API AP_HANDLE APBASE_DECL
ap_Create(int nCameraIndex);

APBASE_API AP_HANDLE APBASE_DECL
ap_CreateFromMidlibCamera(mi_camera_t *pCamera);

APBASE_API AP_HANDLE APBASE_DECL
ap_CreateFromImageFile(const char *szFilename);  //  Image or video file

APBASE_API AP_HANDLE APBASE_DECL
ap_CreateVirtual(const char *szFilename);  //  .xsdat file

APBASE_API void APBASE_DECL
ap_Destroy(AP_HANDLE apbase);

APBASE_API int APBASE_DECL
ap_GetCameraIndex(AP_HANDLE apbase);

APBASE_API const char * APBASE_DECL
ap_GetPartNumber(AP_HANDLE apbase);

APBASE_API ap_u32 APBASE_DECL
ap_GetShipAddr(AP_HANDLE apbase);

APBASE_API void APBASE_DECL
ap_SetShipAddr(AP_HANDLE apbase, ap_u32 nShipAddr);

APBASE_API ap_u32 APBASE_DECL
ap_GetMode(AP_HANDLE apbase, const char *szMode);

APBASE_API ap_s32 APBASE_DECL
ap_SetMode(AP_HANDLE apbase, const char *szMode, ap_u32 nValue);

APBASE_API mi_camera_t * APBASE_DECL
ap_GetMidlibCamera(AP_HANDLE apbase);

APBASE_API ap_s32 APBASE_DECL
ap_LoadIniPreset(AP_HANDLE apbase, const char *szIniFile, const char *szPreset);

APBASE_API ap_s32 APBASE_DECL  //  returns AP_FLAG's
ap_CheckSensorState(AP_HANDLE apbase, unsigned int nCheckFlags);

APBASE_API int APBASE_DECL
ap_GetImageFormat(AP_HANDLE apbase, ap_u32 *pWidth, ap_u32 *pHeight, char *szImageType, mi_u32 nBufferSize);

APBASE_API ap_s32 APBASE_DECL
ap_SetImageFormat(AP_HANDLE apbase, ap_u32 nWidth, ap_u32 nHeight, const char *szImageType);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_GetSensorRegister(AP_HANDLE     apbase,
                     const char *  szRegisterName,
                     const char *  szBitfieldName,
                     ap_u32 *      pnValue,
                     ap_u32        bCached);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_GetSensorRegisterAddr(AP_HANDLE     apbase,
                         mi_addr_type  nAddrType,
                         ap_u32        nAddrSpace,
                         ap_u32        nAddr,
                         ap_u32        nDataBits,
                         ap_u32 *      pnValue,
                         ap_u32        bCached);

APBASE_API ap_s32 APBASE_DECL  //  returns AP_FLAG's
ap_TestSetSensorRegisterAddr(AP_HANDLE       apbase,
                             mi_addr_type    nAddrType,
                             ap_u32          nAddrSpace,
                             ap_u32          nAddr,
                             ap_u32          nDataBits,
                             ap_u32          nNewValue,
                             ap_u32          nOldValue, // if known
                             ap_u32 *        pnOldValue);

APBASE_API ap_s32 APBASE_DECL  //  returns AP_FLAG's
ap_TestSetSensorRegister(AP_HANDLE   apbase,
                         const char *szRegisterName,
                         ap_u32      nNewValue,
                         ap_u32      nOldValue, // if known
                         ap_u32 *    pnOldValue);

APBASE_API ap_s32 APBASE_DECL  //  returns AP_FLAG's
ap_AfterSetSensorRegister(AP_HANDLE      apbase,
                          mi_reg_data_t *pReg,
                          ap_u32         nNewValue,
                          ap_u32         nOldValue);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_SetSensorRegister(AP_HANDLE     apbase,
                     const char *  szRegisterName,
                     const char *  szBitfieldName,
                     ap_u32        nValue,
                     ap_s32 *      pnSideEffects);  //  AP_FLAG's

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_SetSensorRegisterAddr(AP_HANDLE     apbase,
                         mi_addr_type  nAddrType,
                         ap_u32        nAddrSpace,
                         ap_u32        nAddr,
                         ap_u32        nDataBits,
                         ap_u32        nValue,
                         ap_s32        *pnSideEffects);  //  AP_FLAG's

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_GetSensorRegisters(AP_HANDLE     apbase,
                      const char *  szRegisterName,
                      ap_u32        nValues,
                      ap_u32 *      pnValues);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_GetSensorRegistersAddr(AP_HANDLE     apbase,
                          mi_addr_type  nAddrType,
                          ap_u32        nAddrSpace,
                          ap_u32        nAddr,
                          ap_u32        nDataBits,
                          ap_u32        nValues,
                          ap_u32 *      pnValues);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_SetSensorRegisters(AP_HANDLE     apbase,
                      const char *  szRegisterName,
                      ap_u32        nValues,
                      ap_u32 *      pnValues);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_SetSensorRegistersAddr(AP_HANDLE     apbase,
                          mi_addr_type  nAddrType,
                          ap_u32        nAddrSpace,
                          ap_u32        nAddr,
                          ap_u32        nDataBits,
                          ap_u32        nValues,
                          ap_u32 *      pnValues);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_GetRegisters(AP_HANDLE       apbase,
                ap_u32          nBus,
                ap_u32          nDevice,
                ap_u32          nAddr,
                ap_u32          nAddrBits,
                ap_u32          nDataBits,
                ap_u32          nValues,
                ap_u32 *        pnValues);

APBASE_API ap_s32 APBASE_DECL  //  returns midlib error code
ap_SetRegisters(AP_HANDLE       apbase,
                ap_u32          nBus,
                ap_u32          nDevice,
                ap_u32          nAddr,
                ap_u32          nAddrBits,
                ap_u32          nDataBits,
                ap_u32          nValues,
                ap_u32 *        pnValues);

APBASE_API ap_u32 APBASE_DECL
ap_GrabFrame(AP_HANDLE      apbase,
             unsigned char *pBuffer,
             ap_u32         nBufferSize);

APBASE_API unsigned char * APBASE_DECL
ap_ColorPipe(AP_HANDLE      apbase,
             unsigned char *pInBuffer,
             ap_u32         nInBufferSize,
             ap_u32 *       rgbWidth,
             ap_u32 *       rgbHeight,
             ap_u32 *       rgbBitDepth);

APBASE_API int APBASE_DECL
ap_GetState(AP_HANDLE      apbase,
            const char *   szState);

APBASE_API void APBASE_DECL
ap_SetState(AP_HANDLE          apbase,
            const char *       szState,
            int                nValue);

APBASE_API const char * APBASE_DECL
ap_GetStateStr(AP_HANDLE   apbase,
               const char *szState);

APBASE_API void APBASE_DECL
ap_SetStateStr(AP_HANDLE   apbase,
               const char *szState,
               const char *szValue);

APBASE_API ap_s32 APBASE_DECL
ap_SetWindow(AP_HANDLE apbase,
             void *    hWnd,
             int       nViewWidth,
             int       nViewHeight,
             int       bFullScreen);

APBASE_API void APBASE_DECL
ap_RecordStart(AP_HANDLE apbase);

APBASE_API void APBASE_DECL
ap_RecordStop(AP_HANDLE apbase);

APBASE_API int APBASE_DECL
ap_NumRegisters(AP_HANDLE   apbase,
                const char *szDevice);

APBASE_API AP_HANDLE APBASE_DECL
ap_Register(AP_HANDLE   apbase,
            const char *szDevice,
            int         nIndex);

APBASE_API AP_HANDLE APBASE_DECL
ap_FindRegister(AP_HANDLE   apbase,
                const char *szDevice,
                const char *szRegister);

APBASE_API void APBASE_DECL
ap_DestroyRegister(AP_HANDLE apreg);

APBASE_API const char * APBASE_DECL
ap_RegisterSymbol(AP_HANDLE apreg);

APBASE_API const char * APBASE_DECL
ap_RegisterDisplayName(AP_HANDLE apreg);

APBASE_API const char * APBASE_DECL
ap_RegisterDetail(AP_HANDLE apreg);

APBASE_API const char * APBASE_DECL
ap_RegisterLongDesc(AP_HANDLE apreg);

APBASE_API ap_u32 APBASE_DECL
ap_RegisterBitwidth(AP_HANDLE apreg);

APBASE_API ap_u32 APBASE_DECL
ap_RegisterMask(AP_HANDLE apreg);

APBASE_API ap_u32 APBASE_DECL
ap_RegisterDefault(AP_HANDLE apreg);

APBASE_API const char * APBASE_DECL
ap_RegisterDatatype(AP_HANDLE apreg);

APBASE_API double APBASE_DECL
ap_RegisterMinimum(AP_HANDLE apreg);

APBASE_API double APBASE_DECL
ap_RegisterMaximum(AP_HANDLE apreg);

APBASE_API ap_u32 APBASE_DECL
ap_RegisterAddr(AP_HANDLE apreg);

APBASE_API ap_u32 APBASE_DECL
ap_RegisterAddrSpace(AP_HANDLE apreg);

APBASE_API const char * APBASE_DECL
ap_RegisterAddrSpaceId(AP_HANDLE apreg);

APBASE_API const char * APBASE_DECL
ap_RegisterAddrSpaceName(AP_HANDLE apreg);

APBASE_API ap_s32 APBASE_DECL
ap_RegisterAddrType(AP_HANDLE apreg);

APBASE_API ap_u32 APBASE_DECL
ap_RegisterRw(AP_HANDLE apreg);

APBASE_API int APBASE_DECL
ap_NumBitfields(AP_HANDLE apreg);

APBASE_API AP_HANDLE APBASE_DECL
ap_Bitfield(AP_HANDLE   apreg,
            int         nIndex);

APBASE_API AP_HANDLE APBASE_DECL
ap_BitfieldByName(AP_HANDLE   apreg,
                  const char *szBitfield);

APBASE_API AP_HANDLE APBASE_DECL
ap_FindBitfield(AP_HANDLE   apbase,
                const char *szDevice,
                const char *szRegister,
                const char *szBitfield);

APBASE_API AP_HANDLE APBASE_DECL
ap_ParentRegister(AP_HANDLE atbitf);

APBASE_API void APBASE_DECL
ap_DestroyBitfield(AP_HANDLE apbitf);

APBASE_API const char * APBASE_DECL
ap_BitfieldSymbol(AP_HANDLE apbitf);

APBASE_API const char * APBASE_DECL
ap_BitfieldDisplayName(AP_HANDLE apbitf);

APBASE_API const char * APBASE_DECL
ap_BitfieldDetail(AP_HANDLE apbitf);

APBASE_API const char * APBASE_DECL
ap_BitfieldLongDesc(AP_HANDLE apbitf);

APBASE_API ap_u32 APBASE_DECL
ap_BitfieldBitwidth(AP_HANDLE apbitf);

APBASE_API ap_u32 APBASE_DECL
ap_BitfieldMask(AP_HANDLE apbitf);

APBASE_API ap_u32 APBASE_DECL
ap_BitfieldAdjustedMask(AP_HANDLE apbitf);

APBASE_API ap_u32 APBASE_DECL
ap_BitfieldDefault(AP_HANDLE apbitf);

APBASE_API const char * APBASE_DECL
ap_BitfieldDatatype(AP_HANDLE apbitf);

APBASE_API double APBASE_DECL
ap_BitfieldMinimum(AP_HANDLE apbitf);

APBASE_API double APBASE_DECL
ap_BitfieldMaximum(AP_HANDLE apbitf);

APBASE_API ap_u32 APBASE_DECL
ap_BitfieldRw(AP_HANDLE apbitf);



typedef void            (APBASE_DECL * AP_BeginAccessRegs_PROC)(void *pContext);
typedef void            (APBASE_DECL * AP_EndAccessRegs_PROC  )(void *pContext);
typedef void            (APBASE_DECL * AP_GetSaveDir_PROC     )(void *pContext, char *szBuffer, int nBufferLen);
typedef void            (APBASE_DECL * AP_StateChanged_PROC   )(void *pContext, const char *szState, int nValue);
typedef void            (APBASE_DECL * AP_SetOption_PROC      )(void *pContext, const char *szOption, const char *szValue, int nValue);
typedef int             (APBASE_DECL * AP_MultipleChoice_PROC )(void *pContext, const char *szMessage, const char *szChoices);
typedef void            (APBASE_DECL * AP_LogComment_PROC     )(void *pContext, const char *szComment);
typedef int             (APBASE_DECL * AP_LogDebugTrace_PROC  )(void *pContext, const char *szTrace);
typedef int             (APBASE_DECL * AP_ErrorMessage_PROC   )(void *pContext, const char *szMessage, unsigned int mbType);
typedef void            (APBASE_DECL * AP_ScriptOutput_PROC   )(void *pContext, int nUnit, const char *szString);
typedef const char *    (APBASE_DECL * AP_ScriptInput_PROC    )(void *pContext, int nUnit);
typedef void            (APBASE_DECL * AP_ScriptInputFree_PROC)(void *pContext, const char *szString);

APBASE_API void APBASE_DECL ap_SetCallback_BeginAccessRegs(AP_HANDLE apbase, AP_BeginAccessRegs_PROC pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_EndAccessRegs  (AP_HANDLE apbase, AP_EndAccessRegs_PROC   pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_GetSaveDir     (AP_HANDLE apbase, AP_GetSaveDir_PROC      pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_StateChanged   (AP_HANDLE apbase, AP_StateChanged_PROC    pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_SetOption      (AP_HANDLE apbase, AP_SetOption_PROC       pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_MultipleChoice (AP_HANDLE apbase, AP_MultipleChoice_PROC  pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_LogComment     (AP_HANDLE apbase, AP_LogComment_PROC      pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_LogDebugTrace  (AP_HANDLE apbase, AP_LogDebugTrace_PROC   pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_ErrorMessage   (AP_HANDLE apbase, AP_ErrorMessage_PROC    pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_ScriptOutput   (AP_HANDLE apbase, AP_ScriptOutput_PROC    pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_ScriptInput    (AP_HANDLE apbase, AP_ScriptInput_PROC     pProc, void *pContext);
APBASE_API void APBASE_DECL ap_SetCallback_ScriptInputFree(AP_HANDLE apbase, AP_ScriptInputFree_PROC pProc, void *pContext);

//  AP_ErrorMessageA_PROC mbType parameter, based on Windows MessageBox
enum  //  mbType & 0xF
{
    AP_MSGTYPE_OK                   = 0,
    AP_MSGTYPE_OKCANCEL             = 1,
    AP_MSGTYPE_ABORTRETRYIGNORE     = 2,
    AP_MSGTYPE_YESNOCANCEL          = 3,
    AP_MSGTYPE_YESNO                = 4,
    AP_MSGTYPE_RETRYCANCEL          = 5,
    AP_MSGTYPE_CANCELTRYCONTINUE    = 6,
};
//  AP_ErrorMessageA_PROC return values, based on Windows MessageBox
enum
{
    AP_MSG_OK           =  1,
    AP_MSG_CANCEL       =  2,
    AP_MSG_ABORT        =  3,
    AP_MSG_RETRY        =  4,
    AP_MSG_IGNORE       =  5,
    AP_MSG_YES          =  6,
    AP_MSG_NO           =  7,
    AP_MSG_CLOSE        =  8,
    AP_MSG_HELP         =  9,
    AP_MSG_TRYAGAIN     =  10,
    AP_MSG_CONTINUE     =  11,
};

#ifdef __cplusplus
}
#endif

#endif
