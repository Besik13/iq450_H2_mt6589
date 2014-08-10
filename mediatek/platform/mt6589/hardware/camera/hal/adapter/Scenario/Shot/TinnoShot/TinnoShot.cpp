/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/

#define LOG_TAG "MtkCam/Shot"
//

#include <sys/stat.h>
#include <fcntl.h>

#include <common/CamLog.h>
#include <common/CamTypes.h>
#include <common/hw/hwstddef.h>
//
#include <common/camutils/CamFormat.h>
#include <common/camutils/CamInfo.h>
#include <common/camutils/CameraProfile.h>
//
#include <drv/imem_drv.h>
#include <drv/sensor_hal.h>
//
#include <camshot/ICamShot.h>
#include <camshot/ISingleShot.h>
#include <camshot/ISImager.h>

//
#include <Shot/IShot.h>
//
#include "ImpShot.h"
#include "TinnoShot.h"
//

using namespace android;
using namespace NSShot;
using namespace NSCamShot; 
using namespace NSCamHW;




#define MY_LOGV(fmt, arg...)        CAM_LOGV("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGD(fmt, arg...)        CAM_LOGD("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGI(fmt, arg...)        CAM_LOGI("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGW(fmt, arg...)        CAM_LOGW("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGE(fmt, arg...)        CAM_LOGE("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGA(fmt, arg...)        CAM_LOGA("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
#define MY_LOGF(fmt, arg...)        CAM_LOGF("(%d)(%s)[%s] \n"fmt, ::gettid(), getShotName(), __FUNCTION__, ##arg)
//
#define MY_LOGV_IF(cond, ...)       do { if ( (cond) ) { MY_LOGV(__VA_ARGS__); } }while(0)
#define MY_LOGD_IF(cond, ...)       do { if ( (cond) ) { MY_LOGD(__VA_ARGS__); } }while(0)
#define MY_LOGI_IF(cond, ...)       do { if ( (cond) ) { MY_LOGI(__VA_ARGS__); } }while(0)
#define MY_LOGW_IF(cond, ...)       do { if ( (cond) ) { MY_LOGW(__VA_ARGS__); } }while(0)
#define MY_LOGE_IF(cond, ...)       do { if ( (cond) ) { MY_LOGE(__VA_ARGS__); } }while(0)
#define MY_LOGA_IF(cond, ...)       do { if ( (cond) ) { MY_LOGA(__VA_ARGS__); } }while(0)
#define MY_LOGF_IF(cond, ...)       do { if ( (cond) ) { MY_LOGF(__VA_ARGS__); } }while(0)

/*******************************************************************************
*
*******************************************************************************/
extern "C"

sp<IShot> 
createInstance_TinnoShot(char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId)
{
    sp<IShot>       pShot = NULL;
    sp<TinnoShot>  pImpShot = NULL;
    CAM_LOGD("new TinnoShot");
    pImpShot = new TinnoShot(pszShotName,u4ShotMode,i4OpenId);
    if  ( pImpShot == 0 ) {
        CAM_LOGE("[%s] new TinnoShot \n", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (1.2) initialize Implementator if needed.
    if  ( ! pImpShot->onCreate() ) {
        CAM_LOGE("[%s] TinnoShot onCreate() \n", __FUNCTION__);
        goto lbExit;
    }
    //
    //  (2)   new Interface.
    pShot = new IShot(pImpShot);
    if  ( pShot == 0 ) {
        CAM_LOGE("[%s] FBShot new IShot \nt", __FUNCTION__);
        goto lbExit;
    }
lbExit:
    //
    //  Free all resources if this function fails.
    if  ( pShot == 0 && pImpShot != 0 ) {
        pImpShot->onDestroy();
        pImpShot = NULL;
    }
    //
    return  pShot;
}

/*******************************************************************************
*
*******************************************************************************/
TinnoShot::
TinnoShot(char const*const pszShotName,
    uint32_t const u4ShotMode,
    int32_t const i4OpenId)
    : ImpShot(pszShotName, u4ShotMode, i4OpenId)
{
    MY_LOGD("[TinnoShot] TinnoShot +");
    halSensorDev_e          meSensorDev;
    if (i4OpenId == 0) {
            meSensorDev = SENSOR_DEV_MAIN;
    }
    else if (i4OpenId == 1) {
            meSensorDev == SENSOR_DEV_SUB;
    }
    else {
            meSensorDev == SENSOR_DEV_NONE;
    }
    SensorHal* sensor = SensorHal::createInstance();
    if(sensor)
        sensor->sendCommand(meSensorDev, SENSOR_CMD_GET_SENSOR_TYPE, (int)&meSensorType);
    else
        MY_LOGE("[TinnoShot] Can not get sensor object \n");
    MY_LOGD("[TinnoShot] TinnoShot meSensorType %d \n",meSensorType);
    sensor->destroyInstance();

    MY_LOGD("[TinnoShot] TinnoShot -");
}

/*******************************************************************************
*
*******************************************************************************/
bool
TinnoShot::
onCreate()
{
    MBOOL   ret = MFALSE;
    MINT32  ec = 0;

    mpIMemDrv =  IMemDrv::createInstance();
    if (mpIMemDrv == NULL)
    {
        MY_LOGE("g_pIMemDrv is NULL \n");
        return 0;
    }
    ret = MTRUE;
lbExit:
    if  ( ! ret )
    {
        onDestroy();
    }
    MY_LOGD("[init] rc(%d) \n", ret);
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
void
TinnoShot::
onDestroy()
{
    MY_LOGD("[uninit] in");

    if  (mpIMemDrv)
    {
        mpIMemDrv->destroyInstance();
        mpIMemDrv = NULL;
    }
    mu4W_yuv = 0;
    mu4H_yuv = 0;
    
    MY_LOGD("[uninit] out");
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
onCmd_capture()
{
    MBOOL   ret = MFALSE;

    ret = doCapture();
    if  ( ! ret )
    {
        goto lbExit;
    }
    ret = MTRUE;
lbExit:
    releaseBufs();
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
onCmd_reset()
{
    MY_LOGD("[onCmd_reset] in");
    MBOOL   ret = MTRUE;
    //ret = releaseBufs();
    MY_LOGD("[onCmd_reset] out");
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
onCmd_cancel()
{
    MBOOL   ret = MFALSE;
    return  ret;
}

/******************************************************************************
 *
 ******************************************************************************/
bool
TinnoShot::
sendCommand(
    uint32_t const  cmd,
    uint32_t const  arg1,
    uint32_t const  arg2
)
{
    bool ret = true;
    //
    switch  (cmd)
    {
    //  This command is to reset this class. After captures and then reset,
    //  performing a new capture should work well, no matter whether previous
    //  captures failed or not.
    //
    //  Arguments:
    //          N/A
    case eCmd_reset:
        ret = onCmd_reset();
        break;

    //  This command is to perform capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_capture:
        ret = onCmd_capture();
        break;

    //  This command is to perform cancel capture.
    //
    //  Arguments:
    //          N/A
    case eCmd_cancel:
        onCmd_cancel();
        break;
    //
    default:
        ret = ImpShot::sendCommand(cmd, arg1, arg2);
    }
    //
    return ret;
}


/******************************************************************************
* save the buffer to the file
*******************************************************************************/
static bool
saveBufToFile(char const*const fname, MUINT8 *const buf, MUINT32 const size)
{
    int nw, cnt = 0;
    uint32_t written = 0;

    CAM_LOGD("(name, buf, size) = (%s, %x, %d)", fname, buf, size);
    CAM_LOGD("opening file [%s]\n", fname);
    int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC, S_IRWXU);
    //int fd = ::open(fname, O_RDWR | O_CREAT | O_TRUNC);
    if (fd < 0) {
        CAM_LOGE("failed to create file [%s]: %s", fname, ::strerror(errno));
        return false;
    }

    CAM_LOGD("writing %d bytes to file [%s]\n", size, fname);
    while (written < size) {
        nw = ::write(fd,
                     buf + written,
                     size - written);
        if (nw < 0) {
            CAM_LOGE("failed to write to file [%s]: %s", fname, ::strerror(errno));
            break;
        }
        written += nw;
        cnt++;
    }
    CAM_LOGD("done writing %d bytes to file [%s] in %d passes\n", size, fname, cnt);
    ::close(fd);
    return true;
}

/******************************************************************************
*
*******************************************************************************/

// Mhal_facebeauty ->TinnoShot

MBOOL
TinnoShot::
fgCamShotNotifyCb(MVOID* user, CamShotNotifyInfo const msg)
{
    CAM_LOGD("[fgCamShotNotifyCb] + "); 
    TinnoShot *pVDShot = reinterpret_cast <TinnoShot *>(user); 
    if (NULL != pVDShot) 
    {
        CAM_LOGD("[fgCamShotNotifyCb] call back type %d",msg.msgType);
        if (NSCamShot::ECamShot_NOTIFY_MSG_EOF == msg.msgType) 
        {
            pVDShot->mpShotCallback->onCB_Shutter(true, 0); 
            CAM_LOGD("[fgCamShotNotifyCb] call back done");                                                     
        }
    }
    CAM_LOGD("[fgCamShotNotifyCb] -"); 
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
handleYuvDataCallback(MUINT8* const puBuf, MUINT32 const u4Size)
{
    MY_LOGD("[handleYuvDataCallback could dumpYUV buffer] + (puBuf, size) = (%p, %d)", puBuf, u4Size);
    
    #ifdef Debug_Mode 
    saveBufToFile("/sdcard/yuv.yuv", puBuf, u4Size);
    #endif
    
    return 0;
}


/******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
handlePostViewData(MUINT8* const puBuf, MUINT32 const u4Size)
{
    MY_LOGD("[handlePostViewData] + (puBuf, size) = (%p, %d)", puBuf, u4Size); 
    mpShotCallback->onCB_PostviewDisplay(0, 
                                         u4Size, 
                                         reinterpret_cast<uint8_t const*>(puBuf)
                                        ); 

    MY_LOGD("[handlePostViewData] -"); 
    return  MTRUE;
    }

/******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
handleJpegData(MUINT8* const puJpegBuf, MUINT32 const u4JpegSize, MUINT8* const puThumbBuf, MUINT32 const u4ThumbSize, MUINT32 const Mode)
{
#if 1
    MY_LOGD("[pjhandleJpegData] + (puJpgBuf, jpgSize, puThumbBuf, thumbSize, mode ) = (%p, %d, %p, %d, %d)", puJpegBuf, u4JpegSize, puThumbBuf, u4ThumbSize, Mode); 

    MUINT8 *puExifHeaderBuf = new MUINT8[128 * 1024]; 
    MUINT32 u4ExifHeaderSize = 0; 

    makeExifHeader(eAppMode_PhotoMode, puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 
    MY_LOGD("[handleJpegData] (thumbbuf, size, exifHeaderBuf, size) = (%p, %d, %p, %d)", 
                      puThumbBuf, u4ThumbSize, puExifHeaderBuf, u4ExifHeaderSize); 
    // Jpeg callback 
    if(Mode)
    {        
      mpShotCallback->onCB_CompressedImage(0,
                                         u4JpegSize, 
                                         reinterpret_cast<uint8_t const*>(puJpegBuf),
                                         u4ExifHeaderSize,                       //header size 
                                         puExifHeaderBuf,                    //header buf
                                         0,                       //callback index 
                                         true                     //final image 
                                         ); 
     }
    else
    {
     
	  memcpy((void*)mpJpegImg.virtAddr, puExifHeaderBuf, u4ExifHeaderSize);
	  memcpy((void*)(mpJpegImg.virtAddr+u4ExifHeaderSize), puJpegBuf, u4JpegSize);
	  saveBufToFile("/sdcard/tinnoprocess.jpg", (MUINT8*)mpJpegImg.virtAddr, (u4JpegSize+u4ExifHeaderSize));
    }
    MY_LOGD("[handleJpegData] -"); 

    delete [] puExifHeaderBuf; 
#endif
    return MTRUE; 

}


/******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
fgCamShotDataCb(MVOID* user, CamShotDataInfo const msg)
{
    TinnoShot *pVDShot = reinterpret_cast<TinnoShot *>(user); 
    CAM_LOGD("[fgCamShotDataCb] type %d +" ,msg.msgType);
    if (NULL != pVDShot) 
    {
        if (NSCamShot::ECamShot_DATA_MSG_POSTVIEW == msg.msgType) 
        {
            pVDShot->handlePostViewData( msg.puData, msg.u4Size);  
        }
        else if (NSCamShot::ECamShot_DATA_MSG_JPEG == msg.msgType)
        {
            pVDShot->handleJpegData(msg.puData, msg.u4Size, reinterpret_cast<MUINT8*>(msg.ext1), msg.ext2,1);
        }
        else if (NSCamShot::ECamShot_DATA_MSG_YUV == msg.msgType)
        {
            pVDShot->handleYuvDataCallback(msg.puData, msg.u4Size);
        }       
    }
    CAM_LOGD("[fgCamShotDataCb] -" );
    return MTRUE; 
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
createYUVFrame(IMEM_BUF_INFO Srcbufinfo)
{
    MBOOL  ret = MTRUE;
    MINT32 err = 0;
  //  CPTLog(Event_FBShot_createFullFrame, CPTFlagStart);
    MY_LOGD("[createFullFrame] + \n");
    NSCamShot::ISingleShot *pSingleShot = NSCamShot::ISingleShot::createInstance(eShotMode_TinnoShot, "TinnoShot");
    //
    pSingleShot->init();
    EImageFormat ePostViewFmt = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat));    
    ImgBufInfo rSrcImgInfo;

    rSrcImgInfo.u4ImgWidth = mu4W_yuv;
    rSrcImgInfo.u4ImgHeight = mu4H_yuv;
    rSrcImgInfo.eImgFmt = eImgFmt_YV16;    
    rSrcImgInfo.u4Stride[0] = rSrcImgInfo.u4ImgWidth;
    rSrcImgInfo.u4Stride[1] = rSrcImgInfo.u4ImgWidth >> 1;
    rSrcImgInfo.u4Stride[2] = rSrcImgInfo.u4ImgWidth >> 1;
    rSrcImgInfo.u4BufSize = Srcbufinfo.size;
    rSrcImgInfo.u4BufVA = Srcbufinfo.virtAddr;
    rSrcImgInfo.u4BufPA = Srcbufinfo.phyAddr;
    rSrcImgInfo.i4MemID = Srcbufinfo.memID;
    pSingleShot->registerImgBufInfo(ECamShot_BUF_TYPE_YUV, rSrcImgInfo);
    
    //        
    pSingleShot->enableDataMsg(ECamShot_DATA_MSG_YUV
                                 //| ECamShot_DATA_MSG_JPEG
                                 );
    pSingleShot->enableNotifyMsg(NSCamShot::ECamShot_NOTIFY_MSG_EOF);
    // shot param
    NSCamShot::ShotParam rShotParam(eImgFmt_YV16,         //yuv format
                         mShotParam.mi4PictureWidth,      //picutre width
                         mShotParam.mi4PictureHeight,     //picture height
                         0,                               //picture rotation in jpg
                         0,                               //picture flip
                         ePostViewFmt,
                         mShotParam.mi4PostviewWidth,      //postview width
                         mShotParam.mi4PostviewHeight,     //postview height
                         0,                                //postview rotation
                         0,                                //postview flip
                         mShotParam.mu4ZoomRatio           //zoom
                        );


    // sensor param
        NSCamShot::SensorParam rSensorParam(static_cast<MUINT32>(MtkCamUtils::DevMetaInfo::queryHalSensorDev(getOpenId())),                             //Device ID 
                             ACDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG,         //Scenaio 
                             //ACDK_SCENARIO_ID_CAMERA_PREVIEW,         //Scenaio 
                             10,                                       //bit depth 
                             MFALSE,                                   //bypass delay
                             MFALSE                                   //bypass scenario
                            );
    //
    pSingleShot->setCallbacks(fgCamShotNotifyCb, fgCamShotDataCb, this);
    //
    pSingleShot->setShotParam(rShotParam);
    //
    pSingleShot->startOne(rSensorParam);
    //
    pSingleShot->uninit();
    //
    pSingleShot->destroyInstance();

     if  ( ! ret )
        {
            goto lbExit;
        }
	 
	 MY_LOGD("[pjtinno DumpYUV] \n");
    saveBufToFile("/sdcard/img.bin", (uint8_t*)Srcbufinfo.virtAddr, Srcbufinfo.size); 
    MY_LOGD("[createFullFrame] - \n");
lbExit:
    return  ret;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
createJpegImg(NSCamHW::ImgBufInfo const & rSrcImgBufInfo
      , NSCamShot::JpegParam const & rJpgParm
      , MUINT32 const u4Rot
      , MUINT32 const u4Flip
      , NSCamHW::ImgBufInfo const & rJpgImgBufInfo
      , MUINT32 & u4JpegSize)
{

    MBOOL ret = MTRUE;
	#if 1
    // (0). debug
    MY_LOGD("[createJpegImg] - E.");
    MY_LOGD("[createJpegImg] - rSrcImgBufInfo.eImgFmt=%d", rSrcImgBufInfo.eImgFmt);
    MY_LOGD("[createJpegImg] - u4Rot=%d", u4Rot);
    MY_LOGD("[createJpegImg] - u4Flip=%d", u4Flip);
  //  CPTLog(Event_FBShot_JpegEncodeImg, CPTFlagStart);
    //
    // (1). Create Instance
    NSCamShot::ISImager *pISImager = NSCamShot::ISImager::createInstance(rSrcImgBufInfo);
    if(!pISImager) {
    MY_LOGE("HdrShot::createJpegImg can't get ISImager instance.");
    return MFALSE;
    }
    
    // init setting
    NSCamHW::BufInfo rBufInfo(rJpgImgBufInfo.u4BufSize, rJpgImgBufInfo.u4BufVA, rJpgImgBufInfo.u4BufPA, rJpgImgBufInfo.i4MemID);
    //
    pISImager->setTargetBufInfo(rBufInfo);
    //
    pISImager->setFormat(eImgFmt_JPEG);
    //
    pISImager->setRotation(u4Rot);
    //
    pISImager->setFlip(u4Flip);
    //
    pISImager->setResize(rJpgImgBufInfo.u4ImgWidth, rJpgImgBufInfo.u4ImgHeight);
    //
    pISImager->setEncodeParam(rJpgParm.fgIsSOI, rJpgParm.u4Quality);
    //
    pISImager->setROI(Rect(0, 0, rSrcImgBufInfo.u4ImgWidth, rSrcImgBufInfo.u4ImgHeight));
    //
    pISImager->execute();
    //
    u4JpegSize = pISImager->getJpegSize();

	
    MY_LOGD("[pjtinno createJpegImg] u4JpegSize = %d", u4JpegSize);
    
    pISImager->destroyInstance();
  //  CPTLog(Event_FBShot_JpegEncodeImg, CPTFlagEnd);
    
    MY_LOGD("[init] - X. ret: %d.", ret);
  #endif
    return ret;


}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
createJpegImgWithThumbnail(IMEM_BUF_INFO Srcbufinfo, int u4SrcWidth, int u4SrcHeight, MUINT32 const Mode)
{

    MY_LOGD("[pjtinno createJpegImgWithThumbnail] in");

    MBOOL ret = MTRUE;    
	#if 1

	MUINT32     u4Stride[3];
    u4Stride[0] = mu4W_yuv;
    u4Stride[1] = mu4W_yuv >> 1;
    u4Stride[2] = mu4W_yuv >> 1;
    //mrHdrCroppedResult as rYuvImgBufInfo
    MUINT32         u4ResultSize = Srcbufinfo.size;
    NSCamHW::ImgInfo    rYuvImgInfo(eImgFmt_YV16, u4SrcWidth , u4SrcHeight);
    NSCamHW::BufInfo    rYuvBufInfo(u4ResultSize, (MUINT32)Srcbufinfo.virtAddr, 0, Srcbufinfo.memID);
    MY_LOGD("[pjtinno createJpegImgWithThumbnail]Srcbufinfo.virtAddr = %x  ,u4ResultSize =  %d",(MUINT32)Srcbufinfo.virtAddr,u4ResultSize);

	
    NSCamHW::ImgBufInfo   rYuvImgBufInfo(rYuvImgInfo, rYuvBufInfo, u4Stride);

    MUINT32     u4PosStride[3];
    u4PosStride[0] = mPostviewWidth;
    u4PosStride[1] = mPostviewWidth >> 1;
    u4PosStride[2] = mPostviewWidth >> 1;   
	
    EImageFormat mPostviewFormat = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat));    

    NSCamHW::ImgInfo    rPostViewImgInfo(mPostviewFormat, mPostviewWidth, mPostviewHeight);
    NSCamHW::BufInfo    rPostViewBufInfo(mpPostviewImgBuf.size, (MUINT32)mpPostviewImgBuf.virtAddr, 0, mpPostviewImgBuf.memID);
    NSCamHW::ImgBufInfo   rPostViewImgBufInfo(rPostViewImgInfo, rPostViewBufInfo, u4PosStride);

	
    MUINT32 stride[3];

    //rJpegImgBufInfo
    IMEM_BUF_INFO jpegBuf;
    jpegBuf.size = mu4W_yuv * mu4H_yuv;
    mpIMemDrv->allocVirtBuf(&jpegBuf);
    NSCamHW::ImgInfo    rJpegImgInfo(eImgFmt_JPEG, mu4W_yuv, mu4H_yuv);
    NSCamHW::BufInfo    rJpegBufInfo(jpegBuf.size, jpegBuf.virtAddr, jpegBuf.phyAddr, jpegBuf.memID);
    NSCamHW::ImgBufInfo   rJpegImgBufInfo(rJpegImgInfo, rJpegBufInfo, stride);
    
    //rThumbImgBufInfo
    IMEM_BUF_INFO thumbBuf;
    thumbBuf.size = mJpegParam.mi4JpegThumbWidth * mJpegParam.mi4JpegThumbHeight;
    mpIMemDrv->allocVirtBuf(&thumbBuf);
    NSCamHW::ImgInfo    rThumbImgInfo(eImgFmt_JPEG, mJpegParam.mi4JpegThumbWidth, mJpegParam.mi4JpegThumbHeight);
    NSCamHW::BufInfo    rThumbBufInfo(thumbBuf.size, thumbBuf.virtAddr, thumbBuf.phyAddr, thumbBuf.memID);
    NSCamHW::ImgBufInfo   rThumbImgBufInfo(rThumbImgInfo, rThumbBufInfo, stride);
    
    
    MUINT32 u4JpegSize = 0;
    MUINT32 u4ThumbSize = 0;
    
    NSCamShot::JpegParam yuvJpegParam(mJpegParam.mu4JpegQuality, MFALSE);
    ret = ret && createJpegImg(rYuvImgBufInfo, yuvJpegParam, mShotParam.mi4Rotation, 0 , rJpegImgBufInfo, u4JpegSize);
    
    // (3.1) create thumbnail
    // If postview is enable, use postview buffer,
    // else use yuv buffer to do thumbnail
    if (0 != mJpegParam.mi4JpegThumbWidth && 0 != mJpegParam.mi4JpegThumbHeight)
    {
        NSCamShot::JpegParam rParam(mJpegParam.mu4JpegThumbQuality, MFALSE);
        ret = ret && createJpegImg(rPostViewImgBufInfo, rParam, mShotParam.mi4Rotation, 0, rThumbImgBufInfo, u4ThumbSize);
    }

    #ifdef Debug_Mode // Save Img for debug.
    {
        char szFileName[100];
        
        saveBufToFile("/sdcard/Result.jpg", (uint8_t*)jpegBuf.virtAddr, u4JpegSize);
        MY_LOGD("[createJpegImgWithThumbnail] Save %s done.", szFileName);
        
        saveBufToFile("/sdcard/ThumbImg.jpg", (uint8_t*)thumbBuf.virtAddr, u4ThumbSize);
        MY_LOGD("[createJpegImgWithThumbnail] Save %s done.", szFileName);
    }
    #endif  // Debug_Mode


    // Jpeg callback, it contains thumbnail in ext1, ext2.
    handleJpegData((MUINT8*)rJpegImgBufInfo.u4BufVA, u4JpegSize, (MUINT8*)rThumbImgBufInfo.u4BufVA, u4ThumbSize, Mode);
    

    mpIMemDrv->freeVirtBuf(&jpegBuf);
    mpIMemDrv->freeVirtBuf(&thumbBuf);
    MY_LOGD("[createJpegImgWithThumbnail] out");
	#endif
    return ret;
}

/*******************************************************************************
*
*******************************************************************************/



MBOOL
TinnoShot::
TinnoProcess(IMEM_BUF_INFO Srcbufinfo)
{
    MY_LOGD("[pjTinno Process] in");


#if 1		
	MUINT8 *p = (MUINT8*)Srcbufinfo.virtAddr;			
	MUINT8* end_p = (MUINT8*)Srcbufinfo.virtAddr+ Srcbufinfo.size/5;
		while((p<(end_p)) )
		{
			*p = 0;	
			 p++;
		}
		MY_LOGI("pjtinno process  EXIT WHILE \n");		
		MBOOL  ret = MTRUE;	
		return	ret;
#endif

}


MBOOL
TinnoShot::
doCapture()
{
    MBOOL ret = MFALSE;    

    
    ret = requestBufs();
    ret = createYUVFrame(mpSource);
    ret = TinnoProcess(mpSource);	
	
    MY_LOGD("[TinnoCapture] W*H = %d * %d\n", mu4W_yuv, mu4H_yuv);
    saveBufToFile("/sdcard/imgAfterprocess.bin", (uint8_t*)mpSource.virtAddr, mpSource.size); 
    EImageFormat mPostviewFormat = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat));    

    ret = ImgProcess(mpSource, mu4W_yuv, mu4H_yuv, eImgFmt_YV16, mpPostviewImgBuf, mPostviewWidth, mPostviewHeight, mPostviewFormat);
    ret = handlePostViewData((MUINT8*)mpPostviewImgBuf.virtAddr, mpPostviewImgBuf.size);
    ret = createJpegImgWithThumbnail(mpSource, mu4W_yuv, mu4H_yuv, 1);

    if  ( ! ret )
    {
        MY_LOGI("[Tinno] Capture fail \n");
    }
  
    return ret;
}



/******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
allocMem(IMEM_BUF_INFO &memBuf)
{
    if (mpIMemDrv->allocVirtBuf(&memBuf)) {
        MY_LOGE("g_pIMemDrv->allocVirtBuf() error \n");
        return MFALSE;
    }
    memset((void*)memBuf.virtAddr, 0 , memBuf.size);
    if (mpIMemDrv->mapPhyAddr(&memBuf)) {
        MY_LOGE("mpIMemDrv->mapPhyAddr() error \n");
        return MFALSE;
    }
    return MTRUE;
}

/******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
deallocMem(IMEM_BUF_INFO &memBuf)
{
    if (mpIMemDrv->unmapPhyAddr(&memBuf)) {
        MY_LOGE("m_pIMemDrv->unmapPhyAddr() error");
        return MFALSE;
    }

    if (mpIMemDrv->freeVirtBuf(&memBuf)) {
        MY_LOGE("m_pIMemDrv->freeVirtBuf() error");
        return MFALSE;
    }
    return MTRUE;
}

/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
requestBufs()
{
    MBOOL   ret = MFALSE;
    mu4W_yuv = mShotParam.mi4PictureWidth;
    mu4H_yuv = mShotParam.mi4PictureHeight;
    MY_LOGD("[requestBufs] mu4W_yuv %d mu4H_yuv %d",mu4W_yuv,mu4H_yuv);
	mu4SourceSize=mu4W_yuv*mu4H_yuv*2;
  

   // YUV source buffer
    mpSource.size = mu4SourceSize;
    if(!(allocMem(mpSource)))
    {
        mpSource.size = 0;
        MY_LOGE("[requestBufs] mpSource alloc fail");
        ret = 0;
        return ret;
    }
    //postview buffer
	mPostviewWidth = mShotParam.mi4PostviewWidth;
	mPostviewHeight = mShotParam.mi4PostviewHeight;
    EImageFormat mPostviewFormat = static_cast<EImageFormat>(android::MtkCamUtils::FmtUtils::queryImageioFormat(mShotParam.ms8PostviewDisplayFormat));    
    mpPostviewImgBuf.size = android::MtkCamUtils::FmtUtils::queryImgBufferSize(mShotParam.ms8PostviewDisplayFormat, mPostviewWidth, mPostviewHeight);
    if(!(allocMem(mpPostviewImgBuf)))
    {
        mpPostviewImgBuf.size = 0;
        MY_LOGE("[requestBufs] mpPostviewImgBuf alloc fail");
        ret = 0;
        return ret;
    }
	//jpeg buffer
    mpJpegImg.size = mu4SourceSize;
    if(!(allocMem(mpJpegImg)))
    {
        mpJpegImg.size = 0;
        MY_LOGE("[requestBufs] mpJpegImg alloc fail");
        ret = 0;
        return ret;
    }


	
    ret = MTRUE;
    return  ret;
}


/*******************************************************************************
*
*******************************************************************************/
MBOOL
TinnoShot::
releaseBufs()
{
    if(!(deallocMem(mpSource)))
        return  MFALSE;
    if(!(deallocMem(mpPostviewImgBuf)))
        return  MFALSE;
	if(!(deallocMem(mpJpegImg)))
        return  MFALSE;

    return  MTRUE;
}

MBOOL
TinnoShot::
ImgProcess(IMEM_BUF_INFO Srcbufinfo, MUINT32 srcWidth, MUINT32 srcHeight, EImageFormat srctype, IMEM_BUF_INFO Desbufinfo, MUINT32 desWidth, MUINT32 desHeight, EImageFormat destype) const
{
#if 1
    MY_LOGD("[Resize] srcAdr 0x%x srcWidth %d srcHeight %d desAdr 0x%x desWidth %d desHeight %d ",(MUINT32)Srcbufinfo.virtAddr,srcWidth,srcHeight,(MUINT32)Desbufinfo.virtAddr,desWidth,desHeight);

    ImgBufInfo rSrcImgInfo;
    rSrcImgInfo.u4ImgWidth = srcWidth;
    rSrcImgInfo.u4ImgHeight = srcHeight;
    rSrcImgInfo.eImgFmt = srctype;
    rSrcImgInfo.u4Stride[0] = srcWidth;
    rSrcImgInfo.u4Stride[1] = srcWidth >> 1;
    rSrcImgInfo.u4Stride[2] = srcWidth >> 1;
    rSrcImgInfo.u4BufSize = Srcbufinfo.size;
    rSrcImgInfo.u4BufVA = Srcbufinfo.virtAddr;
    rSrcImgInfo.u4BufPA = Srcbufinfo.phyAddr;
    rSrcImgInfo.i4MemID = Srcbufinfo.memID;

    NSCamShot::ISImager *mpISImager = NSCamShot::ISImager::createInstance(rSrcImgInfo);
    if (mpISImager == NULL)
    {
        MY_LOGE("Null ISImager Obj \n");
        return MFALSE;
    }

    BufInfo rBufInfo(Desbufinfo.size, Desbufinfo.virtAddr, Desbufinfo.phyAddr, Desbufinfo.memID);
    //
    mpISImager->setTargetBufInfo(rBufInfo);
    //
    mpISImager->setFormat(destype);
    //
    mpISImager->setRotation(0);
    //
    mpISImager->setFlip(0);
    //
    mpISImager->setResize(desWidth, desHeight);
    //
    mpISImager->setEncodeParam(1, 90);
    //
    mpISImager->setROI(Rect(0, 0, srcWidth, srcHeight));
    //
    mpISImager->execute();
    #ifdef Debug_Mode
    //if(count==0)
    MUINT32 count = 1;
    {
       MY_LOGD("pj Save resize file");
       char szFileName[100];
       ::sprintf(szFileName, "/sdcard/imgprc_%d_%d_%d_%d_%d.bin", (int)srctype, (int)destype,srcWidth,desWidth,count);
       saveBufToFile(szFileName, (MUINT8*)Desbufinfo.virtAddr, (desWidth*desHeight*2));
       MY_LOGD("Save resize file done");
    }
    count++;
    #endif
    MY_LOGD("[Resize] Out");
	#endif
    return  MTRUE;
}

